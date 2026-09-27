#include <raylib.h>

#include "game.hh"

void Game::update() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("First window!", 190, 200, 20, LIGHTGRAY);
    EndDrawing();
}
