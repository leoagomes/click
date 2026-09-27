#pragma once

#include <raylib.h>

class Game {
  public:
    Game() {
    }

    void update() {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("First window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }
};
