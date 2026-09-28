#pragma once

#include <zep/display.h>

namespace Zep {
class RaylibFont : public ZepFont {
  public:
    RaylibFont(ZepDisplay& display) : ZepFont(display) {}

    virtual void SetPixelHeight(int height) override {}

    virtual NVec2f GetTextSize(const uint8_t* pBegin,
                               const uint8_t* pEnd) const override {}
};
} // namespace Zep
