#pragma once

#include <raylib.h>
#include <zep/display.h>

namespace Zep {
class RaylibFont : public ZepFont {
  public:
    RaylibFont(ZepDisplay& display) : ZepFont(display) {}

    virtual void SetPixelHeight(int height) override {}

    virtual NVec2f GetTextSize(const uint8_t* pBegin,
                               const uint8_t* pEnd) const override {}
};

class RaylibDisplay : public ZepDisplay {
  public:
    RaylibDisplay() {}

    virtual void DrawLine(const NVec2f& start,
                          const NVec2f& end,
                          const NVec4f& color = NVec4f(1.0f),
                          float width         = 1.0f) const override {
        DrawLineEx(
            Vector2{start.x, start.y}, Vector2{end.x, end.y}, width, Color{});
    }

    virtual void DrawChars(ZepFont& font,
                           const NVec2f& pos,
                           const NVec4f& col,
                           const uint8_t* text_begin,
                           const uint8_t* text_end = nullptr) const override {}

    virtual void
    DrawRectFilled(const NRectf& a,
                   const NVec4f& col = NVec4f(1.0f)) const override {}

    virtual void SetClipRect(const NRectf& rc) override {}

    virtual ZepFont& GetFont(ZepTextType type) override {}
};
} // namespace Zep
