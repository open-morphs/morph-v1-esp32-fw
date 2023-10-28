#pragma once
#include "AbstractState.hpp"

typedef struct {
    AbstractState* currentState;
    uint8_t signal;
    AbstractState* newState;
}FSMTransition_t;