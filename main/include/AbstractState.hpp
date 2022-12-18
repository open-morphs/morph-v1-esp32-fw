#pragma once
#include <string>

struct AbstractState {
 
    virtual ~AbstractState() = default;
    virtual void enter() = 0;
    virtual uint8_t process() = 0;
    virtual void exit() = 0;

    virtual const char* name() = 0;

    void setSignal(const uint8_t signal) {
        _incomingSignal = signal;
    }

protected:
    uint8_t _incomingSignal = 0;
};