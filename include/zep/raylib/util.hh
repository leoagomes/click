#pragma once

#include <raylib.h>
#include <zep/mcommon/math/math.h>

namespace Zep::Raylib::utils {
struct ClipScope {
    bool _enabled;

    explicit ClipScope(const Zep::NRectf& rect)
        : _enabled(rect.Width() != 0.0f) {
        if (!_enabled)
            return;

        auto left   = static_cast<int>(std::floor(rect.Left()));
        auto top    = static_cast<int>(std::floor(rect.Top()));
        auto right  = static_cast<int>(std::floor(rect.Right()));
        auto bottom = static_cast<int>(std::floor(rect.Bottom()));

        BeginScissorMode(
            left, top, std::max(0, right - left), std::max(0, bottom - top));
    }

    ~ClipScope() {
        if (!_enabled)
            return;

        EndScissorMode();
    }
};
} // namespace Zep::Raylib::utils
