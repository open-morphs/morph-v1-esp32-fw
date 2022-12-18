#pragma once

#include "stdint.h"
#include <functional>

enum class eButtonState {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED = 1,
    BUTTON_UNKNOWN = 2,
};

struct IButton {
    using buttonCallback = std::function<void (void* args)>;

    virtual ~IButton() = default;
    virtual eButtonState buttonState() const = 0;
    virtual bool isPressed() const = 0;
    virtual bool isReleased() const = 0;
    virtual bool isHeldForMs(const uint32_t timeMs) const = 0;
    virtual bool isReleasedForMs(const uint32_t timeMs) const = 0;
    virtual void onPressed(buttonCallback callBack) = 0;
    virtual void onReleased(buttonCallback callBack) = 0;
    virtual void onHeldForMs(buttonCallback callBack, const uint32_t heldPeriodMs) = 0;
    virtual void init() = 0;
};