#include "vosimsynth/widgets/GainUnitWidget.h"
#include "vosimsynth/widgets/CircuitWidget.h"
#include "vosimsynth/UI.h"
#include <vosimlib/Unit.h>
#include <nanogui/theme.h>

using nanogui::Color;

synui::GainUnitWidget::GainUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : SummerUnitWidget(a_parent, a_vm, a_unitId)
{
    
}

void synui::GainUnitWidget::draw(NVGcontext* ctx)
{
    Vector2i mousePos = screen()->mousePos() - absolutePosition();
    float handleRadius = m_handleRadiusRatio*size().x()*0.5;
    bool handleSelected = isHandleSelected(mousePos);

    nvgSave(ctx);

    nvgTranslate(ctx, mPos.x(), mPos.y());
    Color bgColor = theme()->get("/GainUnitWidget/bg-color", Color{127, 32, 11, 255});
    Color handleColor = theme()->get("/GainUnitWidget/fg-color", Color{32, 33, 68, 255});
    Color bgHighlightColor(25, 50);
    Color handleStrokeColor(0,55);
    Color handleHighlightColor(225, 178);
    Color plusColor(255, 255, 235, 255);
    Color oColor(187, 193, 29, 255);
    Color iColor(19, 80, 130, 255);

    nvgBeginPath(ctx);
    nvgEllipse(ctx, size().x() * 0.5, size().y() * 0.5, size().x() * 0.5, size().y() * 0.5);    
    nvgFillColor(ctx, bgColor);
    nvgFill(ctx);
    if (contains(mousePos + position()) && !handleSelected)
    {
        nvgFillColor(ctx, bgHighlightColor);
        nvgFill(ctx);        
    }    

    // Draw connected input ports
    for (int i = 0; i < getUnit_().numInputs(); i++)
    {
        int a_portId = getUnit_().inputs().ids()[i];
        if(getUnit_().isConnected(a_portId)){
            nvgBeginPath(ctx);
            Vector2i portPos = getInputPortAbsPosition(a_portId) - absolutePosition();
            nvgCircle(ctx, portPos.x(), portPos.y(), 2);
            nvgFillColor(ctx, iColor);
            nvgFill(ctx);
        }
    }
    nvgBeginPath(ctx);
    Vector2i portPos = getOutputPortAbsPosition(0) - absolutePosition();
    nvgCircle(ctx, portPos.x(), portPos.y(), 2);
    nvgFillColor(ctx, oColor);
    nvgFill(ctx);

    // Draw inner circle (handle)
    nvgBeginPath(ctx);
    nvgEllipse(ctx, size().x() * 0.5, size().y() * 0.5, handleRadius, handleRadius);
    nvgFillColor(ctx, handleColor);
    nvgFill(ctx);
    if (handleSelected) {
        nvgFillColor(ctx, handleHighlightColor);
        nvgFill(ctx);
    }
    nvgStrokeColor(ctx, handleStrokeColor);
    nvgStrokeWidth(ctx, 5.0f);
    nvgStroke(ctx);    

    // Draw X sign
    nvgBeginPath(ctx);
    nvgTranslate(ctx, size().x()*0.5, size().y()*0.5);
    nvgRotate(ctx, DSP_PI*0.25f);
    nvgMoveTo(ctx, 0.0f, -handleRadius);
    nvgLineTo(ctx, 0.0f, handleRadius);
    nvgMoveTo(ctx, -handleRadius, 0.0f);
    nvgLineTo(ctx, handleRadius, 0.0f);
    nvgStrokeColor(ctx, plusColor);
    nvgStrokeWidth(ctx, 2.0f);
    nvgStroke(ctx);
    
    /* Draw highlight if enabled. */
    if (highlighted())
    {
        drawRadialShadow(ctx, 0, 0, width()*0.5f, 0.0f, 2.0f, { 0.32f, 0.9f, 0.9f, 0.9f }, { 0.0f, 0.0f });
    }

    nvgRestore(ctx);

}
