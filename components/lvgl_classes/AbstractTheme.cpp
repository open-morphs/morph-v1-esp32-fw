#include "AbstractTheme.hpp"

// LVGLStyle AbstractTheme::_bgStyle {};
// LVGLStyle AbstractTheme::_borderStyle {};
// LVGLStyle AbstractTheme::_outlineStyle {};
// LVGLStyle AbstractTheme::_rectShapeStyle;

void AbstractTheme::init() {

    fillBgStyle(_bgStyle);
    fillBorderStyles(_borderStyle);
    fillIndicatorBgStyles(_indicatorStyle);
    fillKnobBgStyles(_knobStyle);
    fillOutlineStyles(_outlineStyle);
    fillShapeStyles(_rectShapeStyle);
    fillTabButtonStyle(_tabBtnsStyle);
    fillTextStyle(_textStyle);
    fillContainerPadStyle(_containerPaddingStyle);
    fillUnlockerKnobStyle(_unlockerKnobStyle);
}
