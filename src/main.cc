#include "rlImGui.h"
#include <spdlog/spdlog.h>
#include <raylib.h>
#include <physfs.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "game.hh"

Game game;

void update_draw_frame();

static constexpr int window_width  = 1280;
static constexpr int window_height = 720;
static constexpr int target_fps = 60;
static constexpr const char* window_name = "(click)";

int main(int argc, char* argv[]) {
    if (!PHYSFS_init(argv[0])) {
        spdlog::error("failed to initialize physfs: {}", PHYSFS_getLastError());
        return -1;
    }
    InitWindow(window_width, window_height, window_name);
    rlImGuiSetup(true);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
    SetTargetFPS(target_fps);

    while (!WindowShouldClose())
        update_draw_frame();
#endif

    rlImGuiShutdown();
    CloseWindow();
    if (!PHYSFS_deinit()) {
        spdlog::error("failed to deinit physfs: {}", PHYSFS_getLastError());
        return -1;
    }
    return 0;
}

void update_draw_frame() {
    game.update();
}
