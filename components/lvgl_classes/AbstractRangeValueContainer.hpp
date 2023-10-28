#pragma once

#include "stdint.h"
#include "AbstractValueChangable.hpp"
#include <math.h>

template<typename ValueType> 
class AbstractRangeValueContainer : public AbstractValueChangable<ValueType> {
    public:
        explicit AbstractRangeValueContainer(const ValueType minValue, const ValueType maxValue, const ValueType step) 
            :   _minValue{minValue},
                _maxValue{maxValue},
                _step{static_cast<ValueType>(std::fabs(step))} {

        }
        // getters
        ValueType step() const { return _step; }
        ValueType minValue() const { return _minValue; }
        ValueType maxValue() const { return _maxValue; }
        // int value() const { return _currentValue; }
        // setters
        // void setValue(const int value);
        void setValue(const ValueType value) override {
            if (value > _maxValue) {
                this->_value = _maxValue;
                return;
            }

            if (value < _minValue) {
                this->_value = _minValue;
                return;
            }

            const int remainder{(value - _minValue)%_step};

            if (remainder) {
                if (remainder >= _step/2.0) {
                    this->_value = value + _step - remainder;
                }
                else {
                    this->_value = value - remainder;
                }
            }
            else {
                this->_value = value;
            }
        }

        void setRange(const ValueType min, const ValueType max) {
            _minValue = min;
            _maxValue = max;
        }
        void setMinValue(const ValueType min) {
            _minValue =  min;
        }
        void setMaxValue(const ValueType max) {
            _maxValue = max;
        }
        void setStep(const ValueType step) {
            _step = std::fabs(step);
        }

        void increment() {
            this->_value += _step;
            if (this->_value > _maxValue){
                this->_value = _maxValue;
            }
        }

        void decrement() {
            this->_value -= _step;
            if (this->_value < _minValue){
                this->_value = _minValue;
            }
        }

        AbstractRangeValueContainer& operator++() {
            increment();
            return *this;
        }

        AbstractRangeValueContainer& operator--() {
            decrement();
            return *this;
        }
    protected:
    private:

        ValueType _minValue;
        ValueType _maxValue;
        ValueType _step;
};
