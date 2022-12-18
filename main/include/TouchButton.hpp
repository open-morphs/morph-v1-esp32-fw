#pragma once

#include "AbstractButton.hpp"
#include "driver/touch_pad.h"

struct TouchButtonData_t;

class TouchButton : public AbstractButton {
    public:
        TouchButton(const TouchButton&);
        TouchButton &operator = (const TouchButton&);
        explicit TouchButton (const touch_pad_t touchPad, const double sensitivity, const uint32_t deBouncePeriodMs = 200);
        ~TouchButton();
    private:
        bool initProcedure() override final;
        eButtonState readState() const override final;
        static esp_err_t globalTouchPadDriverInit();
        void updateBaseLine(const uint16_t value) const;
    private:
        const touch_pad_t _touchPad;
        const double _sensitivity;
        TouchButtonData_t* _touchData;
};