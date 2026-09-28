#pragma once

#include <raylib.h>
#include <zep/display.h>

namespace click {

// Borrows the font: destroy the editor before UnloadFont()/CloseWindow().
class ZepFontRaylib final : public Zep::ZepFont {
public:
    ZepFontRaylib(Zep::ZepDisplay& display, Font font, int pixel_height);
    void SetPixelHeight(int height) override;
    Zep::NVec2f GetTextSize(const uint8_t* begin,
                           const uint8_t* end = nullptr) const override;
    Font font() const { return font_; }
    float advance(int codepoint) const;

private:
    Font font_;
};

// Screen-space renderer: use inside BeginDrawing()/EndDrawing(), outside
// camera modes and other scissor scopes. Input/clipboard are separate adapters.
class ZepDisplayRaylib final : public Zep::ZepDisplay {
public:
    explicit ZepDisplayRaylib(Font font, int pixel_height = 18);

    void DrawLine(const Zep::NVec2f& start, const Zep::NVec2f& end,
                  const Zep::NVec4f& color, float width) const override;
    void DrawChars(Zep::ZepFont& font, const Zep::NVec2f& position,
                   const Zep::NVec4f& color, const uint8_t* begin,
                   const uint8_t* end = nullptr) const override;
    void DrawRectFilled(const Zep::NRectf& rect,
                        const Zep::NVec4f& color) const override;
    void SetClipRect(const Zep::NRectf& rect) override;
    Zep::ZepFont& GetFont(Zep::ZepTextType type) override;

private:
    Font font_;
    int pixel_height_;
    Zep::NRectf clip_{};
};

} // namespace click
