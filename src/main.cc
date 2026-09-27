#include <raylib.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "game.hh"

Game game;

void update_draw_frame();

static constexpr int window_width  = 1280;
static constexpr int window_height = 720;
static constexpr int target_fps = 60;

int main(int argc, char* argv[]) {
    InitWindow(window_width, window_height, "(click)");

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
    SetTargetFPS(target_fps);

    while (!WindowShouldClose())
        update_draw_frame();
#endif

    CloseWindow();

    return 0;
}

void update_draw_frame() {
    game.update();
}
