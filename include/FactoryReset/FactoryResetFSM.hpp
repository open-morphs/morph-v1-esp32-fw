#pragma once

#include "AbstractFSM.hpp"
#include "ITask.hpp"
#include "SingletonTemplate.hpp"
#include "lvgl_classes.h"

struct FactoryResetFSM : AbstractFSM, Singleton<FactoryResetFSM>, ITask {
    friend class Singleton;

    void run(void *args) override;

    void setInitialState(AbstractState *state) {
        _initState = state;
    }

    void onStop() override;

private:
    FactoryResetFSM();

    AbstractState *_initState = nullptr;
};

#define FACTORY_RESET_FSM FactoryResetFSM::instance()
