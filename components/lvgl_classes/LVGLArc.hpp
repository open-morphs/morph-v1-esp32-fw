#pragma once

#include "AbstractRangeValueContainer.hpp"
#include "LVGLBase.hpp"
#include "LVGLLabel.hpp"

class LVGLArc : public AbstractValueChangable<int>, public LVGLBase {

    public:
        eLvglType type() const override {return eLvglType::LVGL_CLASS_ARC;}
        explicit LVGLArc(const bool adjustable, const size_t startAngle,
            const size_t endAngle, const std::string& label, LVGLBase* const parent);

        explicit LVGLArc(const bool adjustable ,const size_t startAngle,
            const size_t endAngle, const std::string& label, lv_obj_t* const parent);
        void setValue(const int value) override;
    private:
        void init();
        void redrawText();
        void changed() override;
    private:

        std::string _valueString;
        LVGLLabel* _valueLbl;
        bool _isAdjustable;
        size_t _startAngle;
        size_t _endAngle;
};