#pragma once

#include "zep/raylib/util.hh"
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
        utils::ClipScope clip{_clip};
        DrawLineEx(Vector2{start.x, start.y},
                   Vector2{end.x, end.y},
                   width,
                   rl_color(color));
    }

    virtual void DrawChars(ZepFont& font,
                           const NVec2f& position,
                           const NVec4f& color,
                           const uint8_t* begin,
                           const uint8_t* end = nullptr) const override {
        utils::ClipScope clip{_clip};

        auto& ray_font = static_cast<Raylib::Font&>(font);
        Vector2 cursor{position.x, position.y};

        if (!begin)
            return;
        if (!end)
            begin = begin + std::strlen(reinterpret_cast<const char*>(begin));
        const std::string text(reinterpret_cast<const char*>(begin),
                               static_cast<size_t>(end - begin));
        for (size_t offset = 0; offset < text.size();) {
            int bytes = 0;
            const int codepoint =
                GetCodepointNext(text.c_str() + offset, &bytes);
            // "visit"
            if (codepoint == '\n') {
                cursor.x = position.x;
                cursor.y = font.GetPixelHeight();
            } else {
                DrawTextCodepoint(ray_font.font(),
                                  codepoint,
                                  cursor,
                                  static_cast<float>(font.GetPixelHeight()),
                                  rl_color(color));
                cursor.x += ray_font.advance();
            }
            offset += std::min(static_cast<size_t>(std::max(1, bytes)),
                               text.size() - offset);
        }
    }

    virtual void
    DrawRectFilled(const NRectf& rect,
                   const NVec4f& color = NVec4f(1.0f)) const override {
        utils::ClipScope clip{_clip};
        DrawRectangle(rect.Left(),
                      rect.Top(),
                      rect.Width(),
                      rect.Height(),
                      rl_color(color));
    }

    virtual void SetClipRect(const NRectf& rect) override {
        _clip = rect;
    }

    virtual ZepFont& GetFont(ZepTextType type) override {}

  private:
    NRectf _clip;
};
} // namespace Zep::Raylib
