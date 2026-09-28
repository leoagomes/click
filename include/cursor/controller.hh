#pragma once

#include <raylib.h>

#include "cursor/button.hh"

namespace click::cursor {
class Controller {
  public:
    Controller()          = default;
    virtual ~Controller() = default;

    const Vector2& position() const {
        return _position;
    }

    virtual void update(float delta) = 0;

    virtual bool pressed(Button button) const  = 0;
    virtual bool down(Button button) const     = 0;
    virtual bool released(Button button) const = 0;
    virtual bool up(Button button) const       = 0;

  protected:
    Vector2 _position;
};
}; // namespace cursor
