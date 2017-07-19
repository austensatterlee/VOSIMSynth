#include "widgets/SummerUnitWidget.h"
#include "vosimsynth/CircuitWidget.h"
#include "vosimsynth/UI.h"
#include <Unit.h>
#include <nanogui/theme.h>
#include <GLFW/glfw3.h>

using Color = nanogui::Color;

synui::SummerUnitWidget::SummerUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : UnitWidget(a_parent, a_vm, a_unitId),
      m_handleRadiusRatio(0.45f) {}

Eigen::Vector2i synui::SummerUnitWidget::getInputPortAbsPosition(int a_portId) {
    return absolutePosition() + Vector2i{0, size().y() * 0.5};
}

Eigen::Vector2i synui::SummerUnitWidget::getOutputPortAbsPosition(int a_portId) {
    return absolutePosition() + Vector2i{size().x(), size().y() * 0.5};
}

int synui::SummerUnitWidget::getInputPort(const Eigen::Vector2i& p) {
    if (!isOutputSelected(p) && !isHandleSelected(p)) {
        const syn::Unit& unit = getUnit_();
        // Find a free input port
        for (int i = 0; i < unit.numInputs(); i++) {
            int inputId = unit.inputs().ids()[i];
            if (unit.inputSource(inputId) == nullptr) {
                return inputId;
            }
        }
    }
    return -1;
}

int synui::SummerUnitWidget::getOutputPort(const Eigen::Vector2i& p) {
    if (isOutputSelected(p))
        return 0;
    return -1;
}

bool synui::SummerUnitWidget::isHandleSelected(const Vector2i& p) const {
    Vector2f center = size().cast<float>() * 0.5f;
    float handleRadius = m_handleRadiusRatio * size().x() * 0.5;
    float distFromCenter = (center - p.cast<float>()).norm();
    return distFromCenter <= handleRadius;
}

bool synui::SummerUnitWidget::isOutputSelected(const Eigen::Vector2i& p) const {
    return p.x() > size().x() * 0.5 && !isHandleSelected(p);
}

void synui::SummerUnitWidget::draw(NVGcontext* ctx) {
    Vector2i mousePos = screen()->mousePos() - absolutePosition();
    float handleRadius = m_handleRadiusRatio * size().x() * 0.5;
    bool handleSelected = isHandleSelected(mousePos);

    nvgSave(ctx);

    nvgTranslate(ctx, mPos.x(), mPos.y());
    Color bgColor = theme()->get("/SummerUnitWidget/bgColor", Color{ 140, 73, 191, 255 });
    Color handleColor = theme()->get("/SummerUnitWidget/fgColor", Color{ 3, 88, 88, 255 });
    Color bgHighlightColor(25, 50);
    Color handleStrokeColor(0, 97);
    Color handleHighlightColor(225, 178);
    Color plusColor(255, 255, 235, 255);
    Color oColor(187, 193, 29, 255);
    Color iColor(19, 80, 130, 255);

    nvgBeginPath(ctx);
    nvgEllipse(ctx, size().x() * 0.5, size().y() * 0.5, size().x() * 0.5, size().y() * 0.5);
    nvgFillColor(ctx, bgColor);
    nvgFill(ctx);
    if (contains(mousePos + position()) && !handleSelected) {
        nvgFillColor(ctx, bgHighlightColor);
        nvgFill(ctx);
    }

    // Draw outer circles (ports)
    for (int i = 0; i < getUnit_().numInputs() - 1; i++) {
        int a_portId = getUnit_().inputs().ids()[i];
        if (getUnit_().isConnected(a_portId)) {
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

    // Draw plus sign
    nvgBeginPath(ctx);
    nvgTranslate(ctx, size().x() * 0.5, size().y() * 0.5);
    nvgMoveTo(ctx, 0.0f, -handleRadius);
    nvgLineTo(ctx, 0.0f, handleRadius);
    nvgMoveTo(ctx, -handleRadius, 0.0f);
    nvgLineTo(ctx, handleRadius, 0.0f);
    nvgStrokeColor(ctx, plusColor);
    nvgStrokeWidth(ctx, 2.0f);
    nvgStroke(ctx);

    /* Draw highlight if enabled. */
    if (highlighted()) {
        drawRadialShadow(ctx, 0, 0, width() * 0.5f, 0.0f, 2.0f, {0.32f, 0.9f, 0.9f, 0.9f}, {0.0f, 0.0f});
    }

    nvgRestore(ctx);
}

Eigen::Vector2i synui::SummerUnitWidget::preferredSize(NVGcontext* ctx) const {
    Vector2f pref_size = {m_parentCircuit->gridSpacing() * 2, m_parentCircuit->gridSpacing() * 2};
    return pref_size.cast<int>();
}

bool synui::SummerUnitWidget::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) {
    Vector2i mousePos = p - position();

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (down) {
            triggerEditorCallback();
            return true;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (isHandleSelected(mousePos) && down) {
            if (promptForDelete_()) {
                return true;
            }
        }
    }

    return UnitWidget::mouseButtonEvent(p, button, down, modifiers);
}
