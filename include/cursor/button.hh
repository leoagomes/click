#pragma once

#include <raylib.h>
#include <spdlog/spdlog.h>

namespace cursor {
class Button {
  public:
    enum Value { left, middle, right };

    Button(Value value) : _value(value) {}

    operator Value() const {
        return _value;
    }

    operator int() const {
        switch (_value) {
        case left:
            return MOUSE_BUTTON_LEFT;
        case middle:
            return MOUSE_BUTTON_MIDDLE;
        case right:
            return MOUSE_BUTTON_RIGHT;
        default:
            spdlog::error("invalid cursor::Button value");
            return MOUSE_BUTTON_LEFT;
        }
    }

  private:
    Value _value;
};
} // namespace cursor
