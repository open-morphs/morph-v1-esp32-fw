#include "AbstractButton.hpp"
#include "esp_log.h"
#include "Utils/TimeUtils.hpp"

static const char* const TAG {"AbstractButton"};
#define BUTTON_TASK_PERIOD_MS   50UL

AbstractButton::AbstractButton(const uint32_t debounceTimeMs)
    :   _isInitialized(false),
        _currentState (eButtonState::BUTTON_UNKNOWN),
        _heldPeriod(0),
        _lastTimeStateChanged(0),
        _debounceTime(debounceTimeMs),
        _onPressedCb(nullptr),
        _onReleasedCb(nullptr),
        _onHeldForCb(nullptr),
        _wasHeldForCallbackCalled(false) {

}

void AbstractButton::init() {
    setName(std::string(TAG));
    setStackSize(configMINIMAL_STACK_SIZE*3);
    setPriority(1);
    setPeriod(BUTTON_TASK_PERIOD_MS);
    if (true == initProcedure()) {
        _isInitialized = true;
    }
}

void AbstractButton::run(void* pArgs) {

    if (false == _isInitialized) {
        ESP_LOGE(TAG, "not initialized yet");
        stop();
    }

    while(1) {
        poll();
        delay(_periodMs);
    }
}

void AbstractButton::poll() {

    const uint32_t now {TimeUtils::nowMs()};
    const eButtonState readButtonState {readState()};
    const bool wasStateChanged {readButtonState != _currentState and true == TimeUtils::isPeriodPassed(_lastTimeStateChanged, _debounceTime)};
    if (true == wasStateChanged) {
        _lastTimeStateChanged = now;
        _currentState = readButtonState;

        switch (_currentState) {
            case eButtonState::BUTTON_PRESSED:
                if (nullptr != _onPressedCb) {
                    _onPressedCb(this);
                }
            break;

            case eButtonState::BUTTON_RELEASED:
                _wasHeldForCallbackCalled = false;
                if (nullptr != _onReleasedCb) {
                    _onReleasedCb(this);
                }
            break;
            default:
                assert(false);
            break;
        }
    }
    if (true == isHeldForMs(_heldPeriod) and false == _wasHeldForCallbackCalled and nullptr != _onHeldForCb) {
        _onHeldForCb(this);
        _wasHeldForCallbackCalled = true;
    }        
}

eButtonState AbstractButton::buttonState() const {
    return _currentState;
}

bool AbstractButton::isPressed() const {
    return (eButtonState::BUTTON_PRESSED == _currentState);
}

bool AbstractButton::isReleased() const {
    return (eButtonState::BUTTON_RELEASED == _currentState);
}

bool AbstractButton::isHeldForMs(const uint32_t timeMs) const {
    return (true == isPressed() and true == isCurrentStateConstFor(timeMs));
}

bool AbstractButton::isReleasedForMs(const uint32_t timeMs) const {
    return (true == isReleased() and true == isCurrentStateConstFor(timeMs));
}

void AbstractButton::onPressed(buttonCallback callBack) {
    _onPressedCb = callBack;
}

void AbstractButton::onReleased(buttonCallback callBack) {
    _onReleasedCb = callBack;
}

void AbstractButton::onHeldForMs(buttonCallback callBack, const uint32_t heldPeriodMs) {
    _onHeldForCb = callBack;
    _heldPeriod = heldPeriodMs;
}

bool AbstractButton::isCurrentStateConstFor(const uint32_t timeMs) const {
    return TimeUtils::isPeriodPassed(_lastTimeStateChanged, timeMs);
}