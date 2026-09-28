#include "zep_example.hh"
#include "zep_raylib_editor.hh"

namespace {
constexpr int editor_font_size = 24;

struct EditorFont {
    Font value = LoadFontEx("data/assets/JetBrainsMono-Regular.ttf", editor_font_size,
                            nullptr, 0);

    EditorFont() {
        SetTextureFilter(value.texture, TEXTURE_FILTER_POINT);
    }
    ~EditorFont() {
        // LoadFontEx returns the shared default font if loading fails.
        if (value.texture.id != GetFontDefault().texture.id)
            UnloadFont(value);
    }
    EditorFont(const EditorFont&) = delete;
    EditorFont& operator=(const EditorFont&) = delete;
};

constexpr const char* sample = R"janet(# Janet / Zep / raylib -- no ImGui renderer or input handling.
# Vim: i inserts, Escape returns to normal mode, hjkl move, u undoes.
# Ctrl+r redoes. Select with v; click to position; y yanks, p pastes.
# System clipboard: "+y and "+p. :q quits. Text is not evaluated.

(defn greet [name]
  (string "Hello, " name "!"))

(each name ["Janet" "Zep" "raylib"]
  (print (greet name)))
)janet";
}

struct ZepExample::Impl {
    // Members are destroyed in reverse order: Zep releases its borrowed font first.
    EditorFont font;
    click::ZepEditorRaylib editor{font.value, editor_font_size};
    Impl() {
        editor.InitWithText("assessment.janet", sample);
        editor.GetConfig().autoHideCommandRegion = false;
    }
};
ZepExample::ZepExample() : impl_(std::make_unique<Impl>()) {}
ZepExample::~ZepExample() = default;
std::string ZepExample::text() const {
    return impl_->editor.GetActiveBuffer()->GetWorkingBuffer().string();
}
bool ZepExample::quit_requested() const { return impl_->editor.quit_requested(); }
void ZepExample::draw() {
    auto& editor = impl_->editor;
    const Rectangle bounds{0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
    editor.SetDisplayRegion({0, 0}, {bounds.width, bounds.height});
    // Zep computes the mouse-to-glyph mapping while drawing. Supply this
    // frame's pointer position before Display, then dispatch click events.
    const auto mouse = GetMousePosition();
    editor.OnMouseMove({mouse.x, mouse.y});
    // Establish layout before mouse hit-testing on the first frame.
    editor.Display();
    editor.HandleInput(bounds);
}
