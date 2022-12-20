#include <Fonts.hpp>
#include <FontTypes.hpp>

static lv_font_t* fontsData[static_cast<size_t>(eFontType::FONT_MAX)] {
    &font_dejavu_14,
    &font_dejavu_18,
    // &font_dejavu_22,
    &font_dejavu_24,
    // &font_dejavu_26,
};

static lv_font_t* japaneseFontsData[static_cast<size_t>(eFontType::FONT_MAX)] {
    &font_sazanami_14,
    &font_sazanami_18,
    // &font_sazanami_22,
    &font_sazanami_24,
    // &font_sazanami_26,
};

lv_font_t** Languages::fonts() {
    return fontsData;
}

lv_font_t** Languages::japaneseFonts() {
    return japaneseFontsData;
}
