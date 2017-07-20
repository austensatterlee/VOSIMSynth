#include "vosimsynth/VOSIMTheme.h"

using nanogui::Color;

synui::VOSIMTheme::VOSIMTheme(NVGcontext* ctx)
    : Theme(ctx) {
    prop("/DefaultUnitWidget/bgColor") = Color(0.3f, 0.9f);
    prop("/DefaultUnitWidget/title/bgColor") = Color(0.325f, 1.0f);
    prop("/DefaultUnitWidget/output/bg-color") = Color(0.24f, 0.16f, 0.09f, 1.0f);
    prop("/DefaultUnitWidget/input/bg-color") = Color(0.09f, 0.16f, 0.24f, 1.0f);
    prop("/DefaultUnitWidget/focused/shadow-size") = 15.0f;
    prop("/DefaultUnitWidget/focused/shadow-feather") = 0.5f;
    prop("/DefaultUnitWidget/focused/shadow-color") = Color(0.32f, 0.9f, 0.9f, 0.9f);
    prop("/DefaultUnitWidget/hovered/shadow-size") = 5.0f;
    prop("/DefaultUnitWidget/hovered/shadow-feather") = 0.46f;
    prop("/DefaultUnitWidget/hovered/shadow-color") = Color(0.8f, 0.5f);

    prop("/SummerUnitWidget/bgColor") = Color(140, 73, 191, 255);
    prop("/SummerUnitWidget/fgColor") = Color(3, 88, 88, 255);
    prop("/GainUnitWidget/bgColor") = Color(127, 32, 11, 255);
    prop("/GainUnitWidget/fgColor") = Color(32, 33, 68, 255);

    prop("/ContextMenu/text-size") = 20;
    prop("/ContextMenu/bgColor") = Color(0.3f, 0.9f);
    prop("/ContextMenu/hoverColor") = Color(0.15f, 1.0f);
}
