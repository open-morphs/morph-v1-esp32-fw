#pragma once

#include "AbstractValueChangable.hpp"

class AbstractSwitch : public AbstractValueChangable<bool>{

    public:
        typedef std::function<void(const bool)> onToggleCbType;
        virtual ~AbstractSwitch() = default;
        void onToggled(onToggleCbType cb) {
            _onToggleCb = cb;
        }
        void toggle() {
            setValue(!_value);
        }
    protected:
        void toggled() {
            toggle();
            if (_onToggleCb) {
                _onToggleCb(_value);
            }
        }
    private:
        onToggleCbType _onToggleCb = nullptr;

};