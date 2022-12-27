#pragma once

#include "LVGLButton.hpp"
#include "LVGLRow.hpp"
#include "LVGLLabel.hpp"
#include "AbstractRangeValueContainer.hpp"
class LVGLNumberBox : public AbstractRangeValueContainer<int>, public LVGLRow {

    public:
        // constructors
        eLvglType type() const override {return eLvglType::LVGL_CLASS_MAX;}
        explicit LVGLNumberBox(const int minValue, const int maxValue, const size_t step, const std::string& label, LVGLBase* const parent);
        explicit LVGLNumberBox(const int minValue, const int maxValue, const size_t step, const std::string& label, lv_obj_t* const parent);
        void setButtonsStyle(const uint8_t part, lv_style_t* const style);
        void setLabelStyle(const uint8_t part, lv_style_t* const style);
        void setSize(const size_t w, const size_t h) override;
        void setValue(const int value) override;
        void setButtonFont(const lv_state_t state, lv_font_t* const font) {
            _leftBtn->setFont(LV_LABEL_PART_MAIN, state, font);
            _rightBtn->setFont(LV_LABEL_PART_MAIN, state, font);
        }

        void setLabelFont(const lv_state_t state, lv_font_t* const font) {
            _valueLbl->setFont(LV_LABEL_PART_MAIN, state, font);
            _valueLbl->setFont(LV_LABEL_PART_MAIN, state, font);
        }

        void setLabelText(const std::string& label) {
            _label = label;
            redrawText();
        }

    private:
        void redrawText();
        void init();
        void changed() override;

        LVGLButton* _leftBtn;
        LVGLLabel* _valueLbl;
        LVGLButton* _rightBtn;
        std::string _label;
};