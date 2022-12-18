#include "LVGLBatteryIndicator.hpp"
#include "battery_full.c"

LV_FONT_DECLARE(empty_battery_icon_font_15);
LV_FONT_DECLARE(empty_battery_icon_font_25);
LV_FONT_DECLARE(empty_battery_icon_font_40);
LV_FONT_DECLARE(empty_battery_icon_font_60);
#define BATTERY_SYMBOL              "\xEF\x89\x84"
#define WIDTH_MULTIPLIER            1.8F
#define CANVAS_WIDTH(W)             (0.58F * W + 0.5F)
#define CANVAS_HEIGHT(H)            (0.40F * H + 0.5F)
#define CANVAS_SHIFT(W)             (W * 0.02)




LVGLBatteryIndicator::LVGLBatteryIndicator(const eBatteryIndicatorSize size, const lv_color_t color, LVGLBase* const parent)
    :   LVGLLabel("", parent),
        _canvasWidth{},
        _canvasHeight{},
        _color{color} {

    size_t width{}, height{};

    switch (size) {
        case eBatteryIndicatorSize::BATTERY_SIZE_SMALL:
            setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &empty_battery_icon_font_15);
            height = 15;
        break;
        case eBatteryIndicatorSize::BATTERY_SIZE_MEDIUM:
            setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &empty_battery_icon_font_25);
            height = 25;
        break;
        case eBatteryIndicatorSize::BATTERY_SIZE_LARGE:
            setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &empty_battery_icon_font_40);
            height = 40;
        break;
        case eBatteryIndicatorSize::BATTERY_SIZE_HUGE:
            setFont(LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &empty_battery_icon_font_60);
            height = 60;
        break;
    
    default:
        break;
    }
    width = height * WIDTH_MULTIPLIER;

    setText(BATTERY_SYMBOL);
    setAlignMode(LV_LABEL_ALIGN_CENTER);

    setSize(width, height);

    _canvasWidth = CANVAS_WIDTH(width);
    _canvasHeight = CANVAS_HEIGHT(height);

    _colorBuff = new lv_color_t[LV_CANVAS_BUF_SIZE_TRUE_COLOR(_canvasWidth, _canvasHeight)];
    _canvas = lv_canvas_create(_obj, NULL);
    lv_canvas_set_buffer(_canvas, _colorBuff, _canvasWidth, _canvasHeight, LV_IMG_CF_TRUE_COLOR);

    lv_obj_align(_canvas, _obj,LV_ALIGN_CENTER, -CANVAS_SHIFT(_canvasWidth), 0);

    applyTheme();

    setChargeLevel(0);
}


void LVGLBatteryIndicator::setChargeLevel(const uint8_t value) {

    const eBatteryState state {stateByValue(value)};


    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_opa = LV_OPA_COVER;

    switch (state) {
        case eBatteryState::BATTERY_EMPTY:
            rect_dsc.bg_color = LV_COLOR_RED;
        break;
        case eBatteryState::BATTERY_25:
            rect_dsc.bg_color = LV_COLOR_ORANGE;
        break;
        case eBatteryState::BATTERY_50:
            rect_dsc.bg_color = LV_COLOR_YELLOW;
        break;
        case eBatteryState::BATTERY_75:
        case eBatteryState::BATTERY_FULL:
            rect_dsc.bg_color = LV_COLOR_LIME;
        break;
    
        default:
        break;
    }

    lv_canvas_fill_bg(_canvas, LV_COLOR_BLACK, LV_OPA_COVER);

    lv_canvas_draw_rect(_canvas, 0, 0, _canvasWidth / 100.0 * value, _canvasHeight, &rect_dsc);
}

eBatteryState LVGLBatteryIndicator::stateByValue(const uint8_t value) const {

    static const uint8_t valueEmpty{12};
    static const uint8_t value25{37};
    static const uint8_t value50{62};
    static const uint8_t value75{87};

    if (value < valueEmpty) {
        return eBatteryState::BATTERY_EMPTY;
    }
    if (value < value25) {
        return eBatteryState::BATTERY_25;
    }
    if (value < value50) {
        return eBatteryState::BATTERY_50;
    }
    if (value < value75) {
        return eBatteryState::BATTERY_75;
    }

    return eBatteryState::BATTERY_FULL;   
}

void LVGLBatteryIndicator::setValue(const uint8_t value) {
    AbstractValueChangable<uint8_t>::setValue(value);
    changed();
}

void LVGLBatteryIndicator::changed() {

    setChargeLevel(value());
    if (_onChangedCb) {
        _onChangedCb(this);
    }
}
