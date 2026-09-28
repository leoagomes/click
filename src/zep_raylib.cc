#include "zep_raylib.hh"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <string>

namespace click {
namespace {
Color ray_color(const Zep::NVec4f& color) {
    auto channel = [](float x) {
        return static_cast<unsigned char>(std::clamp(x, 0.0f, 1.0f) * 255.0f + 0.5f);
    };
    return {channel(color.x), channel(color.y), channel(color.z), channel(color.w)};
}

// Zep supplies byte ranges that need not be NUL terminated. Copying also makes
// raylib's UTF-8 decoder safe at the range boundary. Optimize allocations later.
template <typename Visitor>
void visit_text(const uint8_t* begin, const uint8_t* end, Visitor visit) {
    if (!begin) return;
    if (!end) end = begin + std::strlen(reinterpret_cast<const char*>(begin));
    const std::string text(reinterpret_cast<const char*>(begin),
                           static_cast<size_t>(end - begin));
    for (size_t offset = 0; offset < text.size();) {
        int bytes = 0;
        const int codepoint = GetCodepointNext(text.c_str() + offset, &bytes);
        visit(codepoint);
        offset += std::min(static_cast<size_t>(std::max(1, bytes)), text.size() - offset);
    }
}

// Zep uses an empty rectangle to reset clipping. Each primitive owns its
// scissor scope, so no scissor state leaks into the rest of the game drawing.
struct ClipScope {
    bool enabled;
    explicit ClipScope(const Zep::NRectf& rect) : enabled(rect.Width() != 0.0f) {
        if (enabled) {
            const int left = static_cast<int>(std::floor(rect.Left()));
            const int top = static_cast<int>(std::floor(rect.Top()));
            const int right = static_cast<int>(std::ceil(rect.Right()));
            const int bottom = static_cast<int>(std::ceil(rect.Bottom()));
            BeginScissorMode(left, top, std::max(0, right - left), std::max(0, bottom - top));
        }
    }
    ~ClipScope() { if (enabled) EndScissorMode(); }
};
}

ZepFontRaylib::ZepFontRaylib(Zep::ZepDisplay& display, Font font, int pixel_height)
    : ZepFont(display), font_(font) {
    if (font.baseSize <= 0 || !font.glyphs || !font.recs)
        throw std::invalid_argument("Zep requires a loaded raylib font");
    SetPixelHeight(pixel_height);
}

void ZepFontRaylib::SetPixelHeight(int height) {
    m_pixelHeight = std::max(1, height);
    m_charCache.clear();
    InvalidateCharCache();
}

float ZepFontRaylib::advance(int codepoint) const {
    const int index = GetGlyphIndex(font_, codepoint);
    const float width = font_.glyphs[index].advanceX != 0
        ? static_cast<float>(font_.glyphs[index].advanceX) : font_.recs[index].width + 1.0f;
    return width * static_cast<float>(GetPixelHeight()) / font_.baseSize;
}

Zep::NVec2f ZepFontRaylib::GetTextSize(const uint8_t* begin, const uint8_t* end) const {
    float x = 0, width = 0;
    float height = static_cast<float>(GetPixelHeight());
    visit_text(begin, end, [&](int codepoint) {
        if (codepoint == '\n') {
            width = std::max(width, x);
            x = 0;
            height += GetPixelHeight();
        } else {
            x += advance(codepoint);
        }
    });
    return {std::max(width, x), height};
}

ZepDisplayRaylib::ZepDisplayRaylib(Font font, int pixel_height)
    : font_(font), pixel_height_(std::max(1, pixel_height)) {
    m_defaultTextSize = static_cast<float>(pixel_height_);
    // Validate the borrowed font now, rather than during the first frame.
    GetFont(Zep::ZepTextType::Text);
}

void ZepDisplayRaylib::DrawLine(const Zep::NVec2f& start, const Zep::NVec2f& end,
                               const Zep::NVec4f& color, float width) const {
    ClipScope clip(clip_);
    DrawLineEx({start.x, start.y}, {end.x, end.y}, width, ray_color(color));
}

void ZepDisplayRaylib::DrawChars(Zep::ZepFont& font, const Zep::NVec2f& position,
                                const Zep::NVec4f& color, const uint8_t* begin,
                                const uint8_t* end) const {
    // Fonts assigned to this display must use this renderer's font adapter.
    auto& ray_font = static_cast<ZepFontRaylib&>(font);
    ClipScope clip(clip_);
    Vector2 cursor{position.x, position.y};
    visit_text(begin, end, [&](int codepoint) {
        if (codepoint == '\n') {
            cursor.x = position.x;
            cursor.y += font.GetPixelHeight();
        } else {
            DrawTextCodepoint(ray_font.font(), codepoint, cursor,
                              static_cast<float>(font.GetPixelHeight()), ray_color(color));
            cursor.x += ray_font.advance(codepoint);
        }
    });
}

void ZepDisplayRaylib::DrawRectFilled(const Zep::NRectf& rect,
                                     const Zep::NVec4f& color) const {
    ClipScope clip(clip_);
    DrawRectangleRec({rect.Left(), rect.Top(), rect.Width(), rect.Height()}, ray_color(color));
}

void ZepDisplayRaylib::SetClipRect(const Zep::NRectf& rect) { clip_ = rect; }

Zep::ZepFont& ZepDisplayRaylib::GetFont(Zep::ZepTextType type) {
    auto& font = m_fonts[static_cast<size_t>(type)];
    if (!font) {
        float scale = 1.0f;
        if (type == Zep::ZepTextType::Heading1) scale = 1.75f;
        if (type == Zep::ZepTextType::Heading2) scale = 1.5f;
        if (type == Zep::ZepTextType::Heading3) scale = 1.25f;
        font = std::make_shared<ZepFontRaylib>(*this, font_,
            static_cast<int>(std::round(pixel_height_ * GetPixelScale().y * scale)));
    }
    return *font;
}
} // namespace click
