#pragma once

#include <raylib.h>
#include <zep/display.h>

namespace Zep::Raylib {
inline ::Color rl_color(const NVec4f& color) {
    return ::Color{
        static_cast<unsigned char>(color.x * 255.0f),
        static_cast<unsigned char>(color.y * 255.0f),
        static_cast<unsigned char>(color.z * 255.0f),
        static_cast<unsigned char>(color.w * 255.0f),
    };
}

class Font : public ZepFont {
  public:
    Font(ZepDisplay& display) : ZepFont(display) {}

    virtual void SetPixelHeight(int height) override {}

    virtual NVec2f GetTextSize(const uint8_t* pBegin,
                               const uint8_t* pEnd) const override {}
};

class Display : public ZepDisplay {
  public:
    Display() {}

    virtual void DrawLine(const NVec2f& start,
                          const NVec2f& end,
                          const NVec4f& color = NVec4f(1.0f),
                          float width         = 1.0f) const override {
        DrawLineEx(Vector2{start.x, start.y},
                   Vector2{end.x, end.y},
                   width,
                   rl_color(color));
    }

    virtual void DrawChars(ZepFont& font,
                           const NVec2f& pos,
                           const NVec4f& col,
                           const uint8_t* text_begin,
                           const uint8_t* text_end = nullptr) const override {}

    virtual void
    DrawRectFilled(const NRectf& rect,
                   const NVec4f& color = NVec4f(1.0f)) const override {
        DrawRectangle(rect.Left(),
                      rect.Top(),
                      rect.Width(),
                      rect.Height(),
                      rl_color(color));
    }

    virtual void SetClipRect(const NRectf& rc) override {}

    virtual ZepFont& GetFont(ZepTextType type) override {}
};
} // namespace Zep::Raylib
