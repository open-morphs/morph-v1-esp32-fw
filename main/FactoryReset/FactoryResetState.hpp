#pragma once

#include "AbstractState.hpp"
#include "freertos/event_groups.h"

struct FactoryResetState : AbstractState {

    explicit FactoryResetState (EventGroupHandle_t eventGroup)
        :   _eventGroup{eventGroup} {
    }
protected:
    EventGroupHandle_t _eventGroup;
};