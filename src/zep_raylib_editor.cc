#include "zep_raylib_editor.hh"
#include <zep/mode.h>

namespace click {
namespace {
bool pressed(int key) { return IsKeyPressed(key) || IsKeyPressedRepeat(key); }
struct KeyBinding { int raylib; uint32_t zep; };
constexpr KeyBinding special_keys[] = {
    {KEY_ENTER, Zep::ExtKeys::RETURN}, {KEY_KP_ENTER, Zep::ExtKeys::RETURN},
    {KEY_ESCAPE, Zep::ExtKeys::ESCAPE}, {KEY_BACKSPACE, Zep::ExtKeys::BACKSPACE},
    {KEY_DELETE, Zep::ExtKeys::DEL}, {KEY_TAB, Zep::ExtKeys::TAB},
    {KEY_LEFT, Zep::ExtKeys::LEFT}, {KEY_RIGHT, Zep::ExtKeys::RIGHT},
    {KEY_UP, Zep::ExtKeys::UP}, {KEY_DOWN, Zep::ExtKeys::DOWN},
    {KEY_HOME, Zep::ExtKeys::HOME}, {KEY_END, Zep::ExtKeys::END},
    {KEY_PAGE_UP, Zep::ExtKeys::PAGEUP}, {KEY_PAGE_DOWN, Zep::ExtKeys::PAGEDOWN},
    {KEY_F1, Zep::ExtKeys::F1}, {KEY_F2, Zep::ExtKeys::F2},
    {KEY_F3, Zep::ExtKeys::F3}, {KEY_F4, Zep::ExtKeys::F4},
    {KEY_F5, Zep::ExtKeys::F5}, {KEY_F6, Zep::ExtKeys::F6},
    {KEY_F7, Zep::ExtKeys::F7}, {KEY_F8, Zep::ExtKeys::F8},
    {KEY_F9, Zep::ExtKeys::F9}, {KEY_F10, Zep::ExtKeys::F10},
    {KEY_F11, Zep::ExtKeys::F11}, {KEY_F12, Zep::ExtKeys::F12},
};
}
ZepEditorRaylib::ZepEditorRaylib(Font font, int pixel_height)
    : ZepEditor(new ZepDisplayRaylib(font, pixel_height), Zep::fs::path{},
                Zep::ZepEditorFlags::DisableThreads) { RegisterCallback(this); }
ZepEditorRaylib::~ZepEditorRaylib() { UnRegisterCallback(this); }
Zep::ZepEditor& ZepEditorRaylib::GetEditor() const { return const_cast<ZepEditorRaylib&>(*this); }

void ZepEditorRaylib::Notify(std::shared_ptr<Zep::ZepMessage> message) {
    if (message->messageId == Zep::Msg::GetClipBoard) {
        const char* text = GetClipboardText();
        message->str = text ? text : "";
        message->handled = true;
    } else if (message->messageId == Zep::Msg::SetClipBoard) {
        SetClipboardText(message->str.c_str());
        message->handled = true;
    } else if (message->messageId == Zep::Msg::RequestQuit) {
        quit_requested_ = true;
        message->handled = true;
    }
}

void ZepEditorRaylib::HandleInput(Rectangle bounds) {
    const auto mouse = GetMousePosition();
    const Zep::NVec2f position{mouse.x, mouse.y};
    const bool window_focused = IsWindowFocused();
    const bool hovered = CheckCollisionPointRec(mouse, bounds);
    constexpr int buttons[] = {MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_MIDDLE};
    constexpr Zep::ZepMouseButton zep_buttons[] = {
        Zep::ZepMouseButton::Left, Zep::ZepMouseButton::Right, Zep::ZepMouseButton::Middle};
    if (window_focused && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) focused_ = hovered;
    if (window_focused && (hovered || mouse_down_[0] || mouse_down_[1] || mouse_down_[2]))
        OnMouseMove(position);
    for (size_t i = 0; i < mouse_down_.size(); ++i) {
        if (window_focused && hovered && IsMouseButtonPressed(buttons[i])) {
            mouse_down_[i] = true;
            OnMouseDown(position, zep_buttons[i]);
        }
        // Release outside the editor too, including on application focus loss.
        if (mouse_down_[i] && (!window_focused || !IsMouseButtonDown(buttons[i]))) {
            OnMouseUp(position, zep_buttons[i]);
            mouse_down_[i] = false;
        }
    }
    const float wheel = GetMouseWheelMove();
    if (window_focused && hovered && wheel != 0) OnMouseWheel(position, wheel);
    const bool accept_keys = focused_ && window_focused && GetActiveBuffer();
    uint32_t modifiers = Zep::ModifierKey::None;
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) modifiers |= Zep::ModifierKey::Ctrl;
    if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) modifiers |= Zep::ModifierKey::Alt;
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) modifiers |= Zep::ModifierKey::Shift;
    // Text comes from the character queue; physical keys handle shortcuts and
    // navigation. This avoids inserting a character twice or losing Shift/layout.
    const bool shortcut = (modifiers & (Zep::ModifierKey::Ctrl | Zep::ModifierKey::Alt)) != 0;
    if (accept_keys) {
        auto send = [&](uint32_t key) { GetActiveBuffer()->GetMode()->AddKeyPress(key, modifiers); };
        for (const auto& binding : special_keys)
            if (pressed(binding.raylib)) send(binding.zep);
        if (shortcut) {
            for (int key = KEY_A; key <= KEY_Z; ++key)
                if (pressed(key)) send('a' + key - KEY_A);
            for (int key = KEY_ZERO; key <= KEY_NINE; ++key)
                if (pressed(key)) send('0' + key - KEY_ZERO);
            for (int key : {KEY_SPACE, KEY_APOSTROPHE, KEY_COMMA, KEY_MINUS,
                    KEY_PERIOD, KEY_SLASH, KEY_SEMICOLON, KEY_EQUAL, KEY_LEFT_BRACKET,
                    KEY_BACKSLASH, KEY_RIGHT_BRACKET, KEY_GRAVE})
                if (pressed(key)) send(static_cast<uint32_t>(key));
        }
    }
    for (int ch = GetCharPressed(); ch != 0; ch = GetCharPressed()) {
        if (!accept_keys || shortcut) continue;
        // Upstream AddKeyPress truncates to 8 bits. Don't silently corrupt Unicode.
        if (ch >= 32 && ch <= 126) GetActiveBuffer()->GetMode()->AddKeyPress(ch);
        else if (ch > 126) SetCommandText("ASCII typing supported; use clipboard paste for UTF-8 text.");
    }
}
} // namespace click
