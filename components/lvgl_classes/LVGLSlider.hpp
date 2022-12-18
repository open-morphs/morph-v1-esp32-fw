#pragma once

#include "AbstractRangeValueContainer.hpp"
#include "LVGLBase.hpp"


class LVGLSlider : public AbstractRangeValueContainer<int>, public LVGLBase {
    typedef std::function<void(const int)> onMovedCallbackType;
    public:
        eLvglType type() const override {return eLvglType::LVGL_CLASS_SLIDER;}
        explicit LVGLSlider(const int minValue, const int maxValue, const size_t step, LVGLBase* const parent);
        explicit LVGLSlider(const int minValue, const int maxValue, const size_t step, lv_obj_t* const parent);
        void onMoved(onMovedCallbackType cb);
        void setValue(const int value) override;
    private:
        virtual void init();
        virtual void moved(const int value);
        void changed() override;
    private:
        onMovedCallbackType _onMovedCb = nullptr;
};