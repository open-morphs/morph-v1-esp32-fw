#pragma once

#include "lvgl.h"

LV_FONT_DECLARE(font_dejavu_14);
LV_FONT_DECLARE(font_dejavu_18);
// LV_FONT_DECLARE(font_dejavu_22);
LV_FONT_DECLARE(font_dejavu_24);
// LV_FONT_DECLARE(font_dejavu_26);

LV_FONT_DECLARE(font_sazanami_14);
LV_FONT_DECLARE(font_sazanami_18);
// LV_FONT_DECLARE(font_sazanami_22);
LV_FONT_DECLARE(font_sazanami_24);
// LV_FONT_DECLARE(font_sazanami_26);

namespace Languages {
    lv_font_t** fonts();
    lv_font_t** japaneseFonts();
};