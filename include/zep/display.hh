#pragma once

#include <zep/display.h>

namespace Zep {
class RaylibDisplay : public ZepDisplay {
  public:
    RaylibDisplay() {}

    virtual void DrawLine(const NVec2f& start,
                          const NVec2f& end,
                          const NVec4f& color = NVec4f(1.0f),
                          float width         = 1.0f) const override {
                              DrawLineEx(Vector2{start.x, start.y}, Vector2 endPos, float thick, Color color);
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
