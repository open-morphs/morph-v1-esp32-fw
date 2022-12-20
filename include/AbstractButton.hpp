#pragma once

#include "IButton.hpp"
#include "ITask.hpp"

class AbstractButton :  public IButton, public ITask {

    public:
        explicit AbstractButton(const uint32_t debounceTimeMs = 10);
        eButtonState buttonState() const override final;
        bool isPressed() const override final;
        bool isReleased() const override final;
        bool isHeldForMs(const uint32_t timeMs) const override final;
        bool isReleasedForMs(const uint32_t timeMs) const override final;
        void onPressed(buttonCallback callBack) override final;
        void onReleased(buttonCallback callBack) override final;
        void onHeldForMs(buttonCallback callBack, const uint32_t heldPeriodMs) override final;
        void poll();
        void init() override final;
        void run(void* pArgs) override final;
    protected:
        virtual eButtonState readState() const = 0; // abstracts button state reading procedure
        bool isCurrentStateConstFor(const uint32_t timeMs) const;
        virtual bool initProcedure() = 0;           // abstracts button init procedure
    protected:
        bool _isInitialized;
    private:
        eButtonState _currentState;
        uint32_t _heldPeriod;
        uint32_t _lastTimeStateChanged;
        uint32_t _debounceTime;
        buttonCallback _onPressedCb;
        buttonCallback _onReleasedCb;
        buttonCallback _onHeldForCb;
        bool _wasHeldForCallbackCalled;
};