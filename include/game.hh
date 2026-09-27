#pragma once

#include <janet.h>
#include <raylib.h>

class Game {
  private:
    JanetTable* env = nullptr;

  public:
    Game() {
        env = janet_core_env(nullptr);
    }
    ~Game() {}

    void update() {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("First window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }
};
