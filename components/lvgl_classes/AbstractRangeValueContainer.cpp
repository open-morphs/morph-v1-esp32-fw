// #include "AbstractRangeValueContainer.hpp"

// AbstractRangeValueContainer::AbstractRangeValueContainer(/*lv_obj_t* parent, */const int minValue, const int maxValue, const size_t step)
//     :   /*_innerValueChangedCallback{nullptr},*/
//         /*,
//         _currentValue{minValue},
//         /*_prevValue(minValue)*/ {

// }

// AbstractRangeValueContainer& AbstractRangeValueContainer::operator++() {
//     increment();
//     return *this;
// }

// AbstractRangeValueContainer& AbstractRangeValueContainer::operator--() {
//     decrement();
//     return *this;
// }

// // void AbstractRangeValueContainer::setInnerValueChangedCallback(valueChangedCallback_t cb) {
// //     _innerValueChangedCallback = cb;
// // }

// // void AbstractRangeValueContainer::onValueChanged(valueChangedCallback_t cb) {
// //     _extValueChangedCallback = cb;
// // }

// void AbstractRangeValueContainer::increment() {
//     _value += _step;
//     if (_currentValue > _maxValue){
//         _currentValue = _maxValue;
//     }
//     emitChangedSignal();
// }

// void AbstractRangeValueContainer::decrement() {
//     _currentValue -= _step;
//     if (_currentValue < _minValue){
//         _currentValue = _minValue;
//     }
//     emitChangedSignal();
// }

// void AbstractRangeValueContainer::setValue(const int value) {
//     _currentValue = value;
//     emitChangedSignal();
// }

// void AbstractRangeValueContainer::setRange(const int min, const int max) {
//     _minValue = min;
//     _maxValue = max;
// }

// void AbstractRangeValueContainer::setMinValue(const int min) {
//     _minValue = min;
// }

// void AbstractRangeValueContainer::setMaxValue(const int max) {
//     _maxValue = max;
// }

// void AbstractRangeValueContainer::setStep(const size_t step) {
//     _step = step;
// }

// void AbstractRangeValueContainer::emitChangedSignal() {
//     if (_prevValue != _currentValue) {
//         _prevValue = _currentValue;
//         if (_innerValueChangedCallback) {
//             _innerValueChangedCallback(this, _currentValue);
//         }
//         if (_extValueChangedCallback) {
//             _extValueChangedCallback(this, _currentValue);
//         }
//     }
// }