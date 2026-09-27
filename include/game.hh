#pragma once

#include <memory>
#include <vector>

#include <imgui.h>
#include <janet.h>
#include <raylib.h>
#include <rlImGui.h>

#include "cursor.hh"
#include "cursor/controller/mouse.hh"

class Game {
  private:
    JanetTable* env = nullptr;
    std::vector<Cursor> cursors{};

  public:
    Game() {
        env = janet_core_env(nullptr);
        cursors.push_back(
            Cursor(std::make_shared<cursor::controller::Mouse>()));
    }
    ~Game() {}

    void update(float delta) {
        if (!IsCursorHidden())
            HideCursor();
        for (auto& cursor : cursors) {
            cursor.update(delta);
        }
    }

    void draw() {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawText("First window!", 190, 200, 20, WHITE);

        for (const auto& cursor : cursors) {
            cursor.draw();
        }

        rlImGuiBegin();
        // ImGui::ShowDemoWindow();
        rlImGuiEnd();
        EndDrawing();
    }
};
