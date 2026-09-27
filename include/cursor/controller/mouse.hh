#pragma once

#include <raylib.h>

#include "cursor/controller.hh"

namespace cursor::controller {
class Mouse : public Controller {
  public:
    Mouse() = default;
    ~Mouse() = default;

    virtual void update(float delta) override {
        (void)delta;
        _position = GetMousePosition();
    }

    virtual bool pressed(Button button) const override {
        return IsMouseButtonPressed(button);
    }
    virtual bool down(Button button) const override {
        return IsMouseButtonDown(button);
    }
    virtual bool released(Button button) const override {
        return IsMouseButtonReleased(button);
    }
    virtual bool up(Button button) const override {
        return IsMouseButtonUp(button);
    }
};
} // namespace cursor::controller
