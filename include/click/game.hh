#pragma once

#include <memory>
#include <vector>

#include <imgui.h>
#include <janet.h>
#include <raylib.h>
#include <rlImGui.h>

#include "cursor.hh"
#include "cursor/controller/mouse.hh"
#include "zep/raylib.hh"
// #include "zep_example.hh"

namespace click {
class Game {
  private:
    JanetTable* env = nullptr;
    std::vector<Cursor> cursors{};
    // ZepExample zep_example;

  public:
    Game() {
        env = janet_core_env(nullptr);
        cursors.push_back(
            Cursor(std::make_shared<cursor::controller::Mouse>()));
    }
    ~Game() {}

    void update(float delta) {
        // Use the ordinary mouse for the editor assessment.
        if (IsCursorHidden())
            ShowCursor();
    }

    void draw() {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // zep_example.draw();
        EndDrawing();
    }
    bool quit_requested() const {
        // return zep_example.quit_requested();
        return false;
    }
};
} // namespace click
