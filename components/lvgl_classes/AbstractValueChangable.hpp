#pragma once
#include <functional>

template<typename ValueType> 
class AbstractValueChangable {

    public:
        typedef std::function<void(AbstractValueChangable<ValueType>* obj)> onChangedCallbackType;

        virtual ~AbstractValueChangable() = default;
        AbstractValueChangable()
            :   _value{} {

        }
        virtual void setValue(const ValueType value) {
            _value = value;
        }
        ValueType value() const {
            return _value;
        }
        void onChanged(onChangedCallbackType cb) {
            _onChangedCb = cb;
        }

        virtual void changed() = 0;

    protected:
        ValueType _value;  
        onChangedCallbackType _onChangedCb = nullptr;
};