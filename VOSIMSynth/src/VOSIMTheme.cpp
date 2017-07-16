#include "VOSIMTheme.h"

synui::VOSIMTheme::VOSIMTheme(NVGcontext* ctx)
    : Theme(ctx) {
    prop("/SummerUnitWidget/bgColor") = nanogui::Color(140, 73, 191, 255);
    prop("/SummerUnitWidget/fgColor") = nanogui::Color(3, 88, 88, 255);
    prop("/GainUnitWidget/bgColor") = nanogui::Color(127, 32, 11, 255);
    prop("/GainUnitWidget/fgColor") = nanogui::Color(32, 33, 68, 255);
}

synui::VOSIMTheme::VOSIMTheme(NVGcontext* ctx, const json& j) : Theme(ctx, j) {
    
}
