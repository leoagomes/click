#pragma once

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string_view>

#include <raylib.h>
#include <utf8.h>
#include <zep/display.h>
#include <zep/mode.h>

#include "zep/editor.h"
#include "zep/filesystem.h"
#include "zep/raylib/util.hh"

namespace Zep::Raylib {
// Decode one codepoint, tolerating malformed input. Zep measures every byte
// value 0..255 in isolation when it builds its glyph cache, and buffers may
// contain arbitrary bytes, so an undecodable byte is consumed as U+FFFD
// rather than throwing.
template <typename It> inline char32_t next_codepoint(It& it, It end) {
    try {
        return utf8::next(it, end);
    } catch (const utf8::exception&) {
        ++it; // utfcpp leaves `it` on the offending byte.
        return 0xFFFD;
    }
}

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
    Font(ZepDisplay& display, ::Font font, int pixel_height)
        : ZepFont(display),
          _font(font) {
        if (font.baseSize <= 0 || !font.glyphs || !font.recs)
            throw std::invalid_argument("font not loaded");
        SetPixelHeight(pixel_height);
    }

    virtual void SetPixelHeight(int height) override {
        m_pixelHeight = height;
        m_charCache.clear();
        InvalidateCharCache();
        BuildCharCache();
    }

    virtual NVec2f GetTextSize(const uint8_t* text_begin,
                               const uint8_t* text_end) const override {
        if (!text_end)
            text_end = text_begin
                       + std::strlen(reinterpret_cast<const char*>(text_begin));

        // Matches ImGui's CalcTextSize convention, which Zep assumes: a
        // single line of text is one pixel_height tall, and each newline
        // adds another line.
        float x = 0, width = 0;
        int lines               = 1;
        const auto pixel_height = static_cast<float>(GetPixelHeight());

        std::string_view view{reinterpret_cast<const char*>(text_begin),
                              reinterpret_cast<const char*>(text_end)};
        auto it = view.begin();
        while (it != view.end()) {
            auto codepoint = next_codepoint(it, view.end());
            if (codepoint == '\n') {
                width = std::max(width, x);
                x     = 0;
                ++lines;
            } else {
                x += advance(codepoint);
            }
        }

        return NVec2f{std::max(width, x),
                      static_cast<float>(lines) * pixel_height};
    }

    const ::Font& font() const {
        return _font;
    }

    const float advance(char32_t codepoint) const {
        const auto index           = GetGlyphIndex(_font, codepoint);
        const auto glyph_advance_x = _font.glyphs[index].advanceX;
        const auto width = glyph_advance_x ? static_cast<float>(glyph_advance_x)
                                           : _font.recs[index].width + 1.0f;
        return width * static_cast<float>(GetPixelHeight()) / _font.baseSize;
    }

  private:
    ::Font _font;
};

class Display : public ZepDisplay {
  public:
    Display(::Font font, int pixel_height)
        : _clip(),
          _font(font),
          _pixel_height(pixel_height) {}

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
                           const uint8_t* text_begin,
                           const uint8_t* text_end = nullptr) const override {
        if (!text_begin)
            return;
        if (!text_end)
            text_end = text_begin
                       + std::strlen(reinterpret_cast<const char*>(text_begin));

        utils::ClipScope clip{_clip};
        auto& ray_font = static_cast<Raylib::Font&>(font);
        Vector2 cursor{position.x, position.y};

        auto font_height = font.GetPixelHeight();
        auto ray_color   = rl_color(color);

        std::string_view view{reinterpret_cast<const char*>(text_begin),
                              reinterpret_cast<const char*>(text_end)};
        auto it = view.begin();
        while (it != view.end()) {
            auto codepoint = next_codepoint(it, view.end());
            if (codepoint == '\n') {
                cursor.x = position.x;
                cursor.y = font_height;
            } else {
                DrawTextCodepoint(ray_font.font(),
                                  codepoint,
                                  cursor,
                                  static_cast<float>(font_height),
                                  ray_color);
                cursor.x += ray_font.advance(codepoint);
            }
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

    virtual ZepFont& GetFont(ZepTextType type) override {
        auto& font = m_fonts[static_cast<size_t>(type)];
        if (!font) {
            float scale = 1.0f;
            if (type == Zep::ZepTextType::Heading1)
                scale = 1.75f;
            if (type == Zep::ZepTextType::Heading2)
                scale = 1.5f;
            if (type == Zep::ZepTextType::Heading3)
                scale = 1.25f;
            font = std::make_shared<Raylib::Font>(
                *this,
                _font,
                static_cast<int>(
                    std::round(_pixel_height * GetPixelScale().y * scale)));
        }
        return *font;
    }

  private:
    NRectf _clip;
    ::Font _font;
    int _pixel_height;
};

class Editor final : public Zep::ZepEditor, public Zep::IZepComponent {
  public:
    explicit Editor(::Font font, int pixel_height, IZepFileSystem* fs)
        : _focused(false),
          _quit_requested(false),
          Zep::ZepEditor(new Zep::Raylib::Display(font, pixel_height),
                         Zep::fs::path{},
                         Zep::ZepEditorFlags::DisableThreads,
                         fs) {
        RegisterCallback(this);
    }
    ~Editor() {
        UnRegisterCallback(this);
    }

    void HandleInput(Rectangle bounds) {
        const auto mouse = GetMousePosition();
        const Zep::NVec2f position{mouse.x, mouse.y};
        const bool window_focused = IsWindowFocused();
        const bool hovered        = CheckCollisionPointRec(mouse, bounds);
        constexpr int buttons[]   = {
            MOUSE_BUTTON_LEFT,
            MOUSE_BUTTON_RIGHT,
            MOUSE_BUTTON_MIDDLE,
        };
        constexpr Zep::ZepMouseButton zep_buttons[] = {
            Zep::ZepMouseButton::Left,
            Zep::ZepMouseButton::Right,
            Zep::ZepMouseButton::Middle,
        };
        if (window_focused && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            _focused = hovered;
        if (window_focused
            && (hovered || _mouse_down[0] || _mouse_down[1] || _mouse_down[2]))
            OnMouseMove(position);
        for (size_t i = 0; i < _mouse_down.size(); i++) {
            if (window_focused && hovered && IsMouseButtonPressed(buttons[i])) {
                _mouse_down[i] = true;
                OnMouseDown(position, zep_buttons[i]);
            }
            if (_mouse_down[i]
                && (!window_focused || !IsMouseButtonDown(buttons[i]))) {
                OnMouseUp(position, zep_buttons[i]);
                _mouse_down[i] = false;
            }
        }
        const float wheel = GetMouseWheelMove();
        if (window_focused && hovered && wheel != 0)
            OnMouseWheel(position, wheel);
        const bool accept_keys =
            _focused && window_focused && GetActiveBuffer();
        uint32_t modifiers = Zep::ModifierKey::None;
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
            modifiers |= Zep::ModifierKey::Ctrl;
        if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
            modifiers |= Zep::ModifierKey::Alt;
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            modifiers |= Zep::ModifierKey::Shift;
        const bool shortcut =
            (modifiers & (Zep::ModifierKey::Ctrl | Zep::ModifierKey::Alt)) != 0;
        if (accept_keys) {
            auto send = [&](uint32_t key) {
                GetActiveBuffer()->GetMode()->AddKeyPress(key, modifiers);
            };
            auto is_pressed = [](int key) -> bool {
                return IsKeyPressed(key) || IsKeyPressedRepeat(key);
            };
            for (const auto& binding : special_keys)
                if (is_pressed(binding.raylib))
                    send(binding.zep);
            if (shortcut) {
                for (int key = KEY_A; key <= KEY_Z; key++)
                    if (is_pressed(key))
                        send('a' + key - KEY_A);
                for (int key = KEY_ZERO; key <= KEY_NINE; key++)
                    if (is_pressed(key))
                        send('0' + key - KEY_ZERO);
                for (int key : {KEY_SPACE,
                                KEY_APOSTROPHE,
                                KEY_COMMA,
                                KEY_MINUS,
                                KEY_PERIOD,
                                KEY_SLASH,
                                KEY_SEMICOLON,
                                KEY_EQUAL,
                                KEY_LEFT_BRACKET,
                                KEY_RIGHT_BRACKET,
                                KEY_BACKSLASH,
                                KEY_GRAVE})
                    if (is_pressed(key))
                        send(static_cast<uint32_t>(key));
            }
        }
        for (int ch = GetCharPressed(); ch != 0; ch = GetCharPressed()) {
            if (!accept_keys || shortcut)
                continue;
            if (ch >= 32 && ch <= 126)
                GetActiveBuffer()->GetMode()->AddKeyPress(ch);
            else if (ch > 126)
                SetCommandText("ASCII typing supported; use clipoard paste for "
                               "UTF-8 text.");
        }
    }

    void Notify(std::shared_ptr<Zep::ZepMessage> message) override {
        auto id = message->messageId;
        switch (id) {
        case Zep::Msg::GetClipBoard: {
            const char* text = GetClipboardText();
            message->str     = text ? text : "";
            break;
        }
        case Zep::Msg::SetClipBoard:
            SetClipboardText(message->str.c_str());
            break;
        case Zep::Msg::RequestQuit:
            _quit_requested = true;
            break;
        default:
            message->handled = false;
            return;
        }
        message->handled = true;
    }

    Zep::ZepEditor& GetEditor() const override {
        // TODO: this seems... wrong
        return const_cast<Editor&>(*this);
    }

    bool quit_requested() const {
        return _quit_requested;
    }

  private:
    bool _focused;
    bool _quit_requested;
    std::array<bool, 3> _mouse_down{};

    struct KeyBind {
        int raylib;
        uint32_t zep;
    };
    static constexpr KeyBind special_keys[] = {
        {KEY_ENTER, Zep::ExtKeys::RETURN},
        {KEY_KP_ENTER, Zep::ExtKeys::RETURN},
        {KEY_ESCAPE, Zep::ExtKeys::ESCAPE},
        {KEY_BACKSPACE, Zep::ExtKeys::BACKSPACE},
        {KEY_DELETE, Zep::ExtKeys::DEL},
        {KEY_TAB, Zep::ExtKeys::TAB},
        {KEY_LEFT, Zep::ExtKeys::LEFT},
        {KEY_RIGHT, Zep::ExtKeys::RIGHT},
        {KEY_UP, Zep::ExtKeys::UP},
        {KEY_DOWN, Zep::ExtKeys::DOWN},
        {KEY_HOME, Zep::ExtKeys::HOME},
        {KEY_END, Zep::ExtKeys::END},
        {KEY_PAGE_UP, Zep::ExtKeys::PAGEUP},
        {KEY_PAGE_DOWN, Zep::ExtKeys::PAGEDOWN},
        {KEY_F1, Zep::ExtKeys::F1},
        {KEY_F2, Zep::ExtKeys::F2},
        {KEY_F3, Zep::ExtKeys::F3},
        {KEY_F4, Zep::ExtKeys::F4},
        {KEY_F5, Zep::ExtKeys::F5},
        {KEY_F6, Zep::ExtKeys::F6},
        {KEY_F7, Zep::ExtKeys::F7},
        {KEY_F8, Zep::ExtKeys::F8},
        {KEY_F9, Zep::ExtKeys::F9},
        {KEY_F10, Zep::ExtKeys::F10},
        {KEY_F11, Zep::ExtKeys::F11},
        {KEY_F12, Zep::ExtKeys::F12},
    };
};
} // namespace Zep::Raylib
