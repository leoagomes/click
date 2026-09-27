#pragma once

#include <raylib.h>
#include <chibi/eval.h>

class Game {
  private:
    sexp context;

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
