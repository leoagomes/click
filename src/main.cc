#include <memory>

#include <janet.h>
#include <physfs.h>
#include <raylib.h>
#include <rlImGui.h>
#include <raygui.h>
#include <spdlog/spdlog.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "game.hh"

std::unique_ptr<Game> game{nullptr};

void update_draw_frame();

static constexpr int window_width        = 1280;
static constexpr int window_height       = 720;
static constexpr int target_fps          = 60;
static constexpr const char* window_name = "(click)";


int main(int argc, char* argv[]) {
    janet_init();
    if (!PHYSFS_init(argv[0])) {
        spdlog::error("failed to initialize physfs: {}", PHYSFS_getLastError());
        return -1;
    }
    if (!PHYSFS_mount("data", nullptr, true)) {
        spdlog::error("failed to mount the data path: {}",
                      PHYSFS_getLastError());
        return -1;
    }
    InitWindow(window_width, window_height, window_name);
    SetExitKey(KEY_NULL); // Escape belongs to Zep's Vim mode.

    game = std::make_unique<Game>();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
    // SetTargetFPS(target_fps);

    while (!WindowShouldClose() && !game->quit_requested())
        update_draw_frame();
#endif

    // ensure we're releasing the Game instance before deinitializing everything
    game.reset();

    CloseWindow();
    if (!PHYSFS_deinit()) {
        spdlog::error("failed to deinit physfs: {}", PHYSFS_getLastError());
        return -1;
    }
    janet_deinit();
    return 0;
}

void update_draw_frame() {
    game->update(GetFrameTime());
    game->draw();
}
