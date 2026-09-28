#pragma once
#include "zep_raylib.hh"
#include <array>
#include <zep/editor.h>

namespace click {
// Construct after InitWindow; destroy before unloading the borrowed Font.
class ZepEditorRaylib final : public Zep::ZepEditor, public Zep::IZepComponent {
public:
    explicit ZepEditorRaylib(Font font, int pixel_height = 18);
    ~ZepEditorRaylib();
    void HandleInput(Rectangle bounds);
    void Notify(std::shared_ptr<Zep::ZepMessage> message) override;
    Zep::ZepEditor& GetEditor() const override;
    bool quit_requested() const { return quit_requested_; }
private:
    bool focused_ = true;
    bool quit_requested_ = false;
    std::array<bool, 3> mouse_down_{};
};
} // namespace click
