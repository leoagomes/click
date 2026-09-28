#pragma once

#include <memory>

#include <raylib.hh>
#include <raymath.h>

#include "cursor/button.hh"
#include "cursor/controller.hh"

namespace click {
class Cursor {
  private:
    Texture2D _texture;
    std::shared_ptr<cursor::Controller> _controller;

  public:
    using Button = cursor::Button;

    Cursor(std::shared_ptr<cursor::Controller> controller)
        : _controller(controller) {
        _texture = LoadTextureFromPhysFS("/assets/UI/Cursors/White/Arrow.png");
    }
    ~Cursor() = default;

    const Vector2& position() const {
        return _controller->position();
    }

    inline bool pressed(Button button) const {
        return _controller->pressed(button);
    }

    inline bool down(Button button) const {
        return _controller->down(button);
    }

    inline bool released(Button button) const {
        return _controller->released(button);
    }

    inline bool up(Button button) const {
        return _controller->up(button);
    }

    void update(float delta) {
        return _controller->update(delta);
    }

    void draw() const {
        DrawTextureV(_texture, position(), WHITE);
    }
};
} // namespace click
