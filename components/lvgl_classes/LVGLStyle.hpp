#pragma once

#include "style_data_types.h"

class LVGLStyle {

    public:
        virtual ~LVGLStyle() = default;
        LVGLStyle();
        lv_style_t* style();

        void addShapeStyle(const RectShapeStyle_t& style);
        void addBackgroundStyle(const BackgroundStyle_t& style);
        void addMarginStyle(const MarginStyle_t& style);
        void addPaddingStyle(const PaddingStyle_t& style);
        void addBorderStyle(const BorderStyle_t& style);
        void addOutlineStyle(const OutlineStyle_t& style);        
        void addTextStyle(const TextStyle_t& style);
        void addLineStyle(const LineStyle_t& style);


    private:
        lv_style_t _style;
};