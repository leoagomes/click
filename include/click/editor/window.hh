#pragma once

#include <string>

#include <raygui.h>
#include <raylib.h>

#include "zep/raylib.hh"

namespace click::editor {
class Window {
  public:
    bool visible() const {
        return _visible;
    }

    bool visible(bool show) {
        _visible = show;
    }

    Rectangle& rect() {
        return _rect;
    }

    void draw() {
        if (!visible())
            return;

        _show = !GuiWindowBox(_rect, _title.c_str()):
    }

  private:
    bool _visible;
    Rectangle _rect;
    std::string _title;
    Zep::Raylib::Editor _editor;
};
} // namespace click::editor
