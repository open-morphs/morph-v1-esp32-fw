#pragma once
#include "FontTypes.hpp"
#include <array>
#include "lvgl.h"

namespace Languages {

    struct FontPack {

        lv_font_t* operator[](const eFontType type) const {
            return _fonts.at(static_cast<size_t>(type));
        }

        explicit FontPack(lv_font_t** data) {
            std::copy(data, data + static_cast<size_t>(eFontType::FONT_MAX), _fonts.begin());
        }

    private:
        std::array<lv_font_t*, static_cast<size_t>(eFontType::FONT_MAX)> _fonts; 
    };
};