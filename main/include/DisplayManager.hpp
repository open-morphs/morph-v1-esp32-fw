#pragma once

#include "ITask.hpp"
#include <functional>

class DisplayManager : public ITask{
    public:
        explicit DisplayManager();
        void setSleepTime(const size_t time);
        void setSleepCb(std::function<void()> cb);
        void setWakeUpCb(std::function<void()> cb);
        // void triggerAcitivity();
    private:
        void run(void* args) override;
        size_t _sleepTime;
        size_t _shutdownTime;
        std::function<void()> _onSleepCb;
        std::function<void()> _onWakeUpCb;
};