#include "SummingUnitWidget.h"
#include "CircuitWidget.h"
#include "UI.h"
#include <DSPMath.h>
#include <Unit.h>

synui::SummingUnitWidget::SummingUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId) : UnitWidget(a_parent, a_vm, a_unitId), m_handleRadiusRatio(0.45f)
{
}

Eigen::Vector2i synui::SummingUnitWidget::getInputPortAbsPosition(int a_portId)
{
    int nInputPorts = getUnit_().numInputs() - 1;
    float angle = DSP_PI*(0.5 + (nInputPorts - a_portId)*(1.0f / (nInputPorts + 1)));
    Vector2f pos = absolutePosition().cast<float>() + size().cast<float>()*0.5 + Vector2f{ cos(angle), -sin(angle) }.cwiseProduct(size().cast<float>())*0.5;
    return pos.cast<int>();
}

Eigen::Vector2i synui::SummingUnitWidget::getOutputPortAbsPosition(int a_portId)
{
    return absolutePosition() + Vector2i{ size().x(), size().y()*0.5 };
}

bool synui::SummingUnitWidget::isHandleSelected(const Vector2i& p) const
{
    Vector2f center = size().cast<float>() * 0.5f;
    float handleRadius = m_handleRadiusRatio * size().x() * 0.5;
    float distFromCenter = (center - p.cast<float>()).norm();
    return distFromCenter <= handleRadius;
}

int synui::SummingUnitWidget::getSelectedInputPort(const Vector2i& p) const
{
    int nInputPorts = getUnit_().numInputs() - 1;
    float dAngle = DSP_PI*1.0f / (nInputPorts + 1);
    Vector2f center = size().cast<float>() * 0.5f;
    Vector2f relMouse = p.cast<float>() - center;
    float mouseAngle = atan2f(relMouse.y(), relMouse.x()) - DSP_PI*0.5;
    if (mouseAngle < 0)
        mouseAngle -= 2 * DSP_PI * floor(mouseAngle / (2.0f*DSP_PI));
    int slice = (mouseAngle - dAngle*0.5) / dAngle;
    return slice >= 0 && slice < nInputPorts ? slice : -1;
}

int synui::SummingUnitWidget::getSelectedOutputPort(const Vector2i& p) const
{
    return p.x() >= size().x()*0.5 ? 0 : -1;
}

void synui::SummingUnitWidget::draw(NVGcontext* ctx)
{
    Vector2i mousePos = screen()->mousePos() - absolutePosition();
    float handleRadius = m_handleRadiusRatio*size().x()*0.5;
    bool handleSelected = isHandleSelected(mousePos);

    nvgSave(ctx);

    nvgTranslate(ctx, mPos.x(), mPos.y());
    nanogui::Color bgColor = nanogui::Color(140, 73, 191, 255);
    nanogui::Color bgHighlightColor(25, 50);
    nanogui::Color handleColor(3, 88, 88, 255);
    nanogui::Color handleStrokeColor(0, 97);
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

    // Draw plus sign
    nvgBeginPath(ctx);
    nvgTranslate(ctx, size().x()*0.5, size().y()*0.5);
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

Eigen::Vector2i synui::SummingUnitWidget::preferredSize(NVGcontext* ctx) const
{
    Vector2f pref_size = { m_parentCircuit->getGridSpacing() * 2, m_parentCircuit->getGridSpacing() * 2 };
    return pref_size.cast<int>();
}

bool synui::SummingUnitWidget::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers)
{
    Vector2i mousePos = p - position();
    bool isOutputSelected = mousePos.x() > size().x()*0.5 && !isHandleSelected(mousePos);
    int selectedPort = isOutputSelected ? 0 : -1;
    const syn::Unit& unit = getUnit_();

    if (isOutputSelected)
    {
        selectedPort = 0;
    }
    else
    {
        const int* portIndices = unit.inputs().ids();
        int selectedInputPort = getSelectedInputPort(mousePos);

        // Use the port selected by the mouse if it is free, otherwise find a free one.
        if (selectedInputPort >= 0 && unit.inputSource(portIndices[selectedInputPort]) == nullptr) {
            selectedPort = selectedInputPort;
        }
        else {
            for (int i = 0; i < unit.numInputs(); i++)
            {
                int inputId = unit.inputs().ids()[i];
                if (unit.inputSource(inputId) == nullptr)
                {
                    selectedPort = inputId;
                    break;
                }
            }
        }
    }


    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (down && !isHandleSelected(mousePos) && selectedPort >= 0)
        {
            triggerPortDrag_(selectedPort, isOutputSelected);
            return false;
        }
        if (!down && selectedPort >= 0) {
            triggerPortDrop_(selectedPort, isOutputSelected);
            return false;
        }
        if (down)
        {
            triggerEditorCallback();
            return false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (isHandleSelected(mousePos) && down)
        {
            if (promptForDelete_()) {
                return true;
            }
        }
    }

    return UnitWidget::mouseButtonEvent(p, button, down, modifiers);
}