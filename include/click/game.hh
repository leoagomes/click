#pragma once

#include <memory>
#include <vector>

#include <imgui.h>
#include <janet.h>
#include <raylib.h>
#include <rlImGui.h>

#include "click/cursor.hh"
#include "click/cursor/controller/mouse.hh"
#include "click/editor/window.hh"

#include "zep_example.hh"

namespace click {
class Game {
  private:
    JanetTable* _environment = nullptr;
    std::vector<Cursor> _cursors{};
    ZepExample _zep_example;
    editor::Window _window;

  public:
    Game() {
        _environment = janet_core_env(nullptr);
        _cursors.push_back(
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

        _zep_example.draw();
        EndDrawing();
    }
    bool quit_requested() const {
        // return zep_example.quit_requested();
        return false;
    }
};
} // namespace click
