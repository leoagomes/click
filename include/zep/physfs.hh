#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

#include <physfs.h>
#include <spdlog/spdlog.h>
#include <zep/filesystem.h>

namespace Zep::PhysFS {

// An IZepFileSystem that lives entirely inside the PhysFS virtual file
// system. Every path Zep hands us is interpreted as a PhysFS path: "/" is the
// root of the search path, and relative paths are resolved against the
// working directory. Reads come from any mounted archive or directory; writes
// require a write directory to have been set with PHYSFS_setWriteDir.
//
// PhysFS must be initialised before this object is used. A ZepEditor takes
// ownership of the file system passed to it and deletes it on destruction, so
// PhysFS must remain initialised until the editor is destroyed.
class FileSystem : public IZepFileSystem {
  public:
    explicit FileSystem(fs::path config_path = "/")
        : _working_directory("/"), _config_path(normalize(config_path)) {
        if (!IsDirectory(_config_path))
            _config_path = _working_directory;
    }

    std::string Read(const fs::path& file_path) override {
        const auto path   = normalize(file_path).generic_string();
        PHYSFS_File* file = PHYSFS_openRead(path.c_str());
        if (!file) {
            spdlog::error("zep/physfs: cannot open {} for reading: {}", path,
                          last_error());
            return {};
        }
        std::string contents;
        const auto length = PHYSFS_fileLength(file);
        if (length > 0) {
            contents.resize(static_cast<std::size_t>(length));
            const auto read = PHYSFS_readBytes(
                file, contents.data(), static_cast<PHYSFS_uint64>(length));
            if (read < 0) {
                spdlog::error("zep/physfs: read of {} failed: {}", path,
                              last_error());
                contents.clear();
            } else if (read < length) {
                contents.resize(static_cast<std::size_t>(read));
            }
        }
        PHYSFS_close(file);
        return contents;
    }

    bool Write(const fs::path& file_path, const void* data,
               std::size_t size) override {
        const auto path = normalize(file_path).generic_string();
        // PhysFS writes relative to the write dir, so drop the leading "/".
        PHYSFS_File* file = PHYSFS_openWrite(path.c_str() + 1);
        if (!file) {
            spdlog::error("zep/physfs: cannot open {} for writing: {}", path,
                          last_error());
            return false;
        }
        bool ok = true;
        // Opening with size == 0 truncates, which is valid.
        if (size != 0) {
            const auto written = PHYSFS_writeBytes(file, data, size);
            ok = written == static_cast<PHYSFS_sint64>(size);
            if (!ok)
                spdlog::error("zep/physfs: write of {} failed: {}", path,
                              last_error());
        }
        if (!PHYSFS_close(file))
            ok = false;
        return ok;
    }

    fs::path GetConfigPath() const override {
        return _config_path;
    }

    // PhysFS has no notion of a git checkout, but a mounted directory may
    // contain one, so honour the SearchGitRoot flag by walking up from the
    // start path looking for a ".git" directory.
    fs::path GetSearchRoot(const fs::path& start,
                           bool& found_git) const override {
        found_git   = false;
        fs::path dir = normalize(start.empty() ? _working_directory : start);
        if (!IsDirectory(dir))
            dir = dir.parent_path();
        if (dir.empty() || !IsDirectory(dir))
            return _working_directory;
        if (!(_flags & ZepFileSystemFlags::SearchGitRoot))
            return dir;

        for (fs::path probe = dir;; probe = probe.parent_path()) {
            if (IsDirectory(probe / ".git")) {
                found_git = true;
                return probe;
            }
            if (probe.empty() || probe == probe.root_path())
                break;
        }
        return dir;
    }

    const fs::path& GetWorkingDirectory() const override {
        return _working_directory;
    }

    void SetWorkingDirectory(const fs::path& path) override {
        auto normalized = normalize(path);
        if (IsDirectory(normalized))
            _working_directory = std::move(normalized);
    }

    bool MakeDirectories(const fs::path& path) override {
        const auto dir = normalize(path).generic_string();
        // PHYSFS_mkdir creates intermediate directories, relative to the
        // write dir.
        if (!PHYSFS_mkdir(dir.c_str() + 1)) {
            spdlog::error("zep/physfs: mkdir {} failed: {}", dir,
                          last_error());
            return false;
        }
        return true;
    }

    bool IsDirectory(const fs::path& path) const override {
        PHYSFS_Stat st;
        return stat(path, st) && st.filetype == PHYSFS_FILETYPE_DIRECTORY;
    }

    bool IsReadOnly(const fs::path& path) const override {
        PHYSFS_Stat st;
        // Treat unknown files as read-only; a write would fail cleanly anyway.
        return !stat(path, st) || st.readonly != 0;
    }

    bool Exists(const fs::path& path) const override {
        return PHYSFS_exists(normalize(path).generic_string().c_str()) != 0;
    }

    // Depth-first walk mirroring ZepFileSystemCPP::ScanDirectory: fn_scan
    // returns false to abort the whole scan, and may set dont_recurse to skip
    // the children of the directory it was just handed.
    void ScanDirectory(
        const fs::path& path,
        std::function<bool(const fs::path& path, bool& dont_recurse)> fn_scan)
        const override {
        scan(normalize(path), fn_scan);
    }

    bool Equivalent(const fs::path& a, const fs::path& b) const override {
        return normalize(a) == normalize(b);
    }

    fs::path Canonical(const fs::path& path) const override {
        return normalize(path);
    }

    void SetFlags(std::uint32_t flags) override {
        _flags = flags;
    }

  private:
    // Resolve a path to an absolute, lexically-normal PhysFS path in generic
    // ("/"-separated) form, e.g. "/assets/foo.txt". Relative paths are taken
    // against the working directory. There is no real directory to consult,
    // so this is purely lexical.
    fs::path normalize(const fs::path& path) const {
        fs::path p = path.generic_string();
        if (!p.has_root_directory())
            p = _working_directory / p;
        // Drop any Windows root name (e.g. "C:") so PhysFS gets "/x/y".
        p = fs::path("/") / p.relative_path();
        p = p.lexically_normal();
        auto s = p.generic_string();
        while (s.size() > 1 && s.back() == '/')
            s.pop_back();
        return fs::path(s);
    }

    bool stat(const fs::path& path, PHYSFS_Stat& st) const {
        return PHYSFS_stat(normalize(path).generic_string().c_str(), &st) != 0;
    }

    using ScanFn = std::function<bool(const fs::path&, bool&)>;

    struct ScanContext {
        const FileSystem* self;
        const fs::path* dir;
        const ScanFn* fn;
        bool keep_going = true;
    };

    static PHYSFS_EnumerateCallbackResult
    scan_entry(void* data, const char* /*origdir*/, const char* name) {
        auto& ctx         = *static_cast<ScanContext*>(data);
        fs::path entry    = *ctx.dir / name;
        bool dont_recurse = false;
        if (!(*ctx.fn)(entry, dont_recurse)) {
            ctx.keep_going = false;
            return PHYSFS_ENUM_STOP;
        }
        if (!dont_recurse && ctx.self->IsDirectory(entry) &&
            !ctx.self->scan(entry, *ctx.fn)) {
            ctx.keep_going = false;
            return PHYSFS_ENUM_STOP;
        }
        return PHYSFS_ENUM_OK;
    }

    // Returns false when fn_scan asked to stop.
    bool scan(const fs::path& dir, const ScanFn& fn_scan) const {
        ScanContext ctx{this, &dir, &fn_scan};
        PHYSFS_enumerate(dir.generic_string().c_str(), &scan_entry, &ctx);
        return ctx.keep_going;
    }

    static const char* last_error() {
        return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
    }

    fs::path _working_directory;
    fs::path _config_path;
    std::uint32_t _flags = ZepFileSystemFlags::SearchGitRoot;
};

} // namespace Zep::PhysFS
