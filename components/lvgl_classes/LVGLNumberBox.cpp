#include "LVGLNumberBox.hpp"

LVGLNumberBox::LVGLNumberBox(const int minValue, const int maxValue, const size_t step, const std::string& label, lv_obj_t* const parent)
    :   AbstractRangeValueContainer(minValue, maxValue, step),
        LVGLRow(parent),
        _leftBtn{new LVGLButton("-", this)},
        _valueLbl{new LVGLLabel("", this)},
        _rightBtn{new LVGLButton("+", this)},
        _label{label} {
    init();
}

LVGLNumberBox::LVGLNumberBox(const int minValue, const int maxValue, const size_t step,  const std::string& label, LVGLBase* const parent) 
    : AbstractRangeValueContainer(minValue, maxValue, step),
        LVGLRow(parent),
        _leftBtn{new LVGLButton("-", this)},
        _valueLbl{new LVGLLabel("", this)},
        _rightBtn{new LVGLButton("+", this)},
        _label{label} {
    init();
}

void LVGLNumberBox::init() {
    
    setValue(minValue());
   
    _valueLbl->setAlignMode(LV_LABEL_ALIGN_CENTER);

    this->_valueLbl->setText("%d %s", this->value(), _label.c_str());

    _leftBtn->disable();

    auto minusCb = [this](const LVGLButton* btn) {
        decrement();
        changed();
    };

    auto plusCb = [this](const LVGLButton* btn) {
        increment();
        changed();
    };

    _leftBtn->onClicked(minusCb);
    _leftBtn->onLongPressedRepeat(minusCb);

    _rightBtn->onClicked(plusCb);
    _rightBtn->onLongPressedRepeat(plusCb);
}

void LVGLNumberBox::redrawText() {
    _valueLbl->setText("%d\t%s", value(), _label.c_str());
}

void LVGLNumberBox::setButtonsStyle(const uint8_t part, lv_style_t* const style) {
    _leftBtn->setStyle(part, style);
    _rightBtn->setStyle(part, style);
}

void LVGLNumberBox::setLabelStyle(const uint8_t part, lv_style_t* const style) {
    _valueLbl->setStyle(part, style);
}

void LVGLNumberBox::setSize(const size_t w, const size_t h) {

    const size_t btnSize {h};
    _leftBtn->setSize(btnSize, btnSize);
    _rightBtn->setSize(btnSize, btnSize);
    _valueLbl->setSize(w - btnSize * 2.5 , h);
}

void LVGLNumberBox::changed() {
    redrawText();

    this->value() == this->minValue() ?
        this->_leftBtn->disable() :
        this->_leftBtn->enable();

    this->value() == this->maxValue() ?
        this->_rightBtn->disable() :
        this->_rightBtn->enable();

    if (_onChangedCb) {
        _onChangedCb(this);
    }
}

void LVGLNumberBox::setValue(const int value) {
    AbstractRangeValueContainer<int>::setValue(value);
    changed();
}

