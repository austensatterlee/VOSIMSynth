#include "MultiplyingUnitWidget.h"
#include "CircuitWidget.h"
#include "UI.h"
#include <Unit.h>

synui::MultiplyingUnitWidget::MultiplyingUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : SummingUnitWidget(a_parent, a_vm, a_unitId)
{
    
}

void synui::MultiplyingUnitWidget::draw(NVGcontext* ctx)
{
    Vector2i mousePos = screen()->mousePos() - absolutePosition();
    float handleRadius = m_handleRadiusRatio*size().x()*0.5;
    bool handleSelected = isHandleSelected(mousePos);

    nvgSave(ctx);

    nvgTranslate(ctx, mPos.x(), mPos.y());
    nanogui::Color bgColor = nanogui::Color(127, 32, 11, 255);
    nanogui::Color bgHighlightColor(25, 50);
    nanogui::Color handleColor(32, 33, 68, 255);
    nanogui::Color handleStrokeColor(0,55);
    nanogui::Color handleHighlightColor(225, 178);
    nanogui::Color plusColor(255, 255, 235, 255);
    nanogui::Color oColor(187, 193, 29, 255);
    nanogui::Color iColor(19, 80, 130, 255);

    nvgBeginPath(ctx);
    nvgEllipse(ctx, size().x() * 0.5, size().y() * 0.5, size().x() * 0.5, size().y() * 0.5);    
    nvgFillColor(ctx, bgColor);
    nvgFill(ctx);
    if (contains(mousePos + position()) && !handleSelected)
    {
        nvgFillColor(ctx, bgHighlightColor);
        nvgFill(ctx);        
    }    

    // Draw outer circles (ports)
    for (int i = 0; i < getUnit_().numInputs() - 1; i++)
    {
        nvgBeginPath(ctx);
        Vector2i portPos = getInputPortAbsPosition(i) - absolutePosition();
        nvgCircle(ctx, portPos.x(), portPos.y(), 2);
        nvgFillColor(ctx, iColor);
        nvgFill(ctx);
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
