#pragma once
#include "AbstractState.hpp"
#include "FSMTransition.hpp"
#include <vector>

#define SIGNAL_NULL 0

struct AbstractFSM {

    AbstractFSM() = default;

    void setTransitionTable(const std::vector<FSMTransition_t>& table) {
        _transitionsTable = table;
    }

    void setState(AbstractState* const state, const uint8_t signal = SIGNAL_NULL) {
        if (_currentState) {
            _currentState->exit();
        }
        _currentState = state;
        _currentState->setSignal(signal);
        _currentState->enter();
    }

    AbstractState* currentState() const {
        return _currentState;
    }

    uint8_t processCurrentState() {
        if (_currentState) {
            return _currentState->process();
        }
        return SIGNAL_NULL;
    }

    AbstractState* handleSignal(const uint8_t signal) {
        if (!_currentState) {
            return nullptr;
        }

        for (const auto transition : _transitionsTable) {
            if (transition.currentState == _currentState and 
                transition.signal == signal) {
                return transition.newState;
            }
        }
        return nullptr;
    }
private:
    AbstractState* _currentState = nullptr;
    std::vector<FSMTransition_t> _transitionsTable;
};