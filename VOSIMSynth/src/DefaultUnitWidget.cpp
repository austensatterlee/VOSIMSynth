#include "vosimsynth/widgets/DefaultUnitWidget.h"
#include "vosimsynth/widgets/CircuitWidget.h"
#include "vosimsynth/UI.h"
#include "vosimsynth/VOSIMTheme.h"
#include <vosimlib/DSPMath.h>
#include <vosimlib/Unit.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/textbox.h>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>

using nanogui::Color;

synui::DefaultUnitWidget::DefaultUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : UnitWidget(a_parent, a_vm, a_unitId) {
    const syn::Unit& unit = getUnit_();

    const auto& inputs = unit.inputs();
    const auto& outputs = unit.outputs();

    // Create title
    m_titleLabel = new nanogui::Label(this, "", "sans-bold");
    m_titleLabel->setTooltip(unit.getClassName());
    m_titleLabel->setDraggable(false);
    m_titleLabel->setPosition({ 1, 0 });

    m_titleTextBox = new nanogui::TextBox(this, "");
    m_titleTextBox->setEditable(true);
    m_titleTextBox->setVisible(false);
    m_titleTextBox->setAlignment(nanogui::TextBox::Alignment::Center);
    m_titleTextBox->setCallback([this](const string& a_str) {
        if (a_str.empty())
            return false;
        setName(a_str);
        m_titleLabel->setVisible(true);
        m_titleTextBox->setVisible(false);
        m_parentCircuit->updateUnitPos(this, position(), true);
        return true;
    });

    // Create input port labels
    for (int i = 0; i < inputs.size(); i++) {
        int inputId = inputs.ids()[i];
        const string& inputName = inputs.getNameFromId(inputId);
        auto lbl = new nanogui::Label(this, inputName, "sans");
        lbl->setId(std::to_string(inputId));
        lbl->setTooltip(inputName);
        lbl->setDraggable(false);
        m_inputLabels[inputId] = lbl;
    }
    // Create output port labels
    for (int i = 0; i < outputs.size(); i++) {
        int outputId = outputs.ids()[i];
        const string& outputName = outputs.getNameFromId(outputId);
        auto lbl = new nanogui::Label(this, outputName, "sans");
        lbl->setId(std::to_string(outputId));
        lbl->setTooltip(outputName);
        lbl->setDraggable(false);
        m_outputLabels[outputId] = lbl;
    }
}

void synui::DefaultUnitWidget::draw(NVGcontext* ctx) {
    m_titleLabel->setCaption(getName());

    nvgSave(ctx);
    nvgTranslate(ctx, mPos.x(), mPos.y());

    // Draw background
    Color bgColor = theme()->get<Color>("/DefaultUnitWidget/bg-color", {0.3f, 0.9f});
    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, mSize.x(), mSize.y());
    nvgFillColor(ctx, bgColor);
    nvgFill(ctx);

    // Draw title background
    Color titleBgColor = theme()->get<Color>("/DefaultUnitWidget/title/bg-color", {0.325f, 1.0f});
    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, width(), m_titleLabel->height());
    nvgFillColor(ctx, titleBgColor);
    nvgFill(ctx);

    // Draw divisions
    auto drawDivisions = [ctx](const map<int, nanogui::Label*>& lbls, const Color& c1, const Color& c2) {
        int i = 0;
        for (auto& kv : lbls) {
            nvgBeginPath(ctx);
            if (i % 2 == 0)
                nvgFillColor(ctx, c1);
            else
                nvgFillColor(ctx, c2);
            nvgRect(ctx, kv.second->position().x(), kv.second->position().y(), kv.second->width(), kv.second->height());
            nvgFill(ctx);
            i++;
        }
    };

    Color oColor = theme()->get<Color>("/DefaultUnitWidget/output/bg-color", {0.24f, 0.16f, 0.09f, 1.0f});
    Color oColor2 = Color(1.26f, 1.0f).cwiseProduct(oColor);
    Color iColor = theme()->get<Color>("/DefaultUnitWidget/input/bg-color", {0.09f, 0.16f, 0.24f, 1.0f});
    Color iColor2 = Color(1.26f, 1.0f).cwiseProduct(iColor);
    drawDivisions(m_inputLabels, iColor, iColor2);
    drawDivisions(m_outputLabels, oColor, oColor2);

    /* Draw highlight if enabled. */
    if (highlighted()) {
        drawRectShadow(ctx, 0, 0, width(), height(), 1.0f,
            theme()->get("/DefaultUnitWidget/focused/shadow-size", 15.0f),
            theme()->get("/DefaultUnitWidget/focused/shadow-feather", 0.5f),
            theme()->get<Color>("/DefaultUnitWidget/focused/shadow-color", {0.32f, 0.9f, 0.9f, 0.9f}),
            {0.0f, 0.0f}
        );
    }

    /* Handle mouse movements */
    Vector2i mousePos = screen()->mousePos() - absolutePosition();
    if (contains(mousePos + position())) {
        // Highlight on mouse over
        drawRectShadow(ctx, 0, 0, width(), height(),
            1.0f,
            theme()->get("/DefaultUnitWidget/hovered/shadow-size", 5.0f),
            theme()->get("/DefaultUnitWidget/hovered/shadow-feather", 0.46f),
            theme()->get<Color>("/DefaultUnitWidget/hovered/shadow-color", {0.8f, 0.5f}),
            {0.0f, 0.0f});

        if (mousePos.y() < m_titleLabel->height()) {
            // Highlight title
            nvgBeginPath(ctx);
            nvgRect(ctx, 0.0f, 0.0f, width(), m_titleLabel->height());
            nvgFillColor(ctx, Color(1.0f, 0.05f));
            nvgFill(ctx);
        } else {
            // Highlight ports
            for (auto& kv : m_inputLabels) {
                if (kv.second->contains(mousePos)) {
                    const Vector2i& pos = kv.second->position();
                    const Vector2i& size = kv.second->size();
                    nvgBeginPath(ctx);
                    nvgRect(ctx, pos.x(), pos.y(), size.x(), size.y());
                    nvgFillColor(ctx, Color(1.0f, 0.05f));
                    nvgFill(ctx);
                    break;
                }
            }

            for (auto& kv : m_outputLabels) {
                if (kv.second->contains(mousePos)) {
                    const Vector2i& pos = kv.second->position();
                    const Vector2i& size = kv.second->size();
                    nvgBeginPath(ctx);
                    nvgRect(ctx, pos.x(), pos.y(), size.x(), size.y());
                    nvgFillColor(ctx, Color(1.0f, 0.05f));
                    nvgFill(ctx);
                    break;
                }
            }
        }
    }

    // Draw outline
    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, mSize.x(), mSize.y());
    nvgStrokeColor(ctx, Color(0.0f, 0.9f));
    nvgStrokeWidth(ctx, 1.0f);
    nvgStroke(ctx);

    nvgRestore(ctx);

    Widget::draw(ctx);
}

void synui::DefaultUnitWidget::setName(const string& a_name) {
    UnitWidget::setName(a_name);
    m_titleLabel->setCaption(a_name);
}

Eigen::Vector2i synui::DefaultUnitWidget::getInputPortAbsPosition(int a_portId) {
    auto& lbl = m_inputLabels[a_portId];
    return lbl->absolutePosition() + Vector2f{0, lbl->height() * 0.5}.cast<int>();
}

Eigen::Vector2i synui::DefaultUnitWidget::getOutputPortAbsPosition(int a_portId) {
    auto& lbl = m_outputLabels[a_portId];
    return lbl->absolutePosition() + Vector2f{lbl->width(), lbl->height() * 0.5}.cast<int>();
}

int synui::DefaultUnitWidget::getInputPort(const Vector2i& a_pos) {
    int i = 0;
    for (auto& kv : m_inputLabels) {
        if (kv.second->contains(a_pos)) {
            return i;
        }
        i++;
    }
    return -1;
}

int synui::DefaultUnitWidget::getOutputPort(const Vector2i& a_pos) {
    int i = 0;
    for (auto& kv : m_outputLabels) {
        if (kv.second->contains(a_pos)) {
            return i;
        }
        i++;
    }
    return -1;
}

void synui::DefaultUnitWidget::performLayout(NVGcontext* ctx) {
    auto getMaxWidth = [](const auto& a_widgetMap)
    {
        int w = 0;
        for (const auto& kv : a_widgetMap) {
            w = std::max(w, kv.second->width());
        }
        return w;
    };
    Widget::performLayout(ctx);

    const int portHeight = fontSize();
    const int portTop = m_parentCircuit->fixToGrid({ 0,m_titleLabel->preferredSize(ctx).y() }).y();
    const int inWidth = getMaxWidth(m_inputLabels);
    const int outWidth = getMaxWidth(m_outputLabels);

    int y = portTop;
    for (auto& kv : m_inputLabels) {
        auto lbl = kv.second;
        y = m_parentCircuit->fixToGrid({ 0, y }).y();
        lbl->setPosition({ 0, y });
        lbl->setSize({inWidth, portHeight});
        y += portHeight;
    }

    y = portTop;
    for (auto& kv : m_outputLabels) {
        auto lbl = kv.second;
        y = m_parentCircuit->fixToGrid({ 0, y }).y();
        lbl->setPosition({ width() - outWidth, y });
        lbl->setSize({outWidth, portHeight});
        y += portHeight;
    }
}

Eigen::Vector2i synui::DefaultUnitWidget::preferredSize(NVGcontext* ctx) const {
    auto getMaxWidth = [ctx](const auto& a_widgetMap)
    {
        int w = 0;
        for (const auto& kv : a_widgetMap) {
            w = std::max(w, kv.second->preferredSize(ctx).x());
        }
        return w;
    };
    const int gridSpacing = m_parentCircuit->gridSpacing();
    const int portHeight = fontSize();
    const int portTop = m_parentCircuit->fixToGrid({0,m_titleLabel->preferredSize(ctx).y()}).y();
//    const int portTop = (std::floorf(m_titleLabel->preferredSize(ctx).y() * 1.0f / gridSpacing) + 0.5f) * gridSpacing;
    int bottom = portTop;
    for (int i = 0; i < int(std::max(m_inputLabels.size(), m_outputLabels.size())); i++) {
        bottom = m_parentCircuit->fixToGrid({0,bottom}).y();
        bottom += portHeight;
    }
    const int inWidth = getMaxWidth(m_inputLabels);
    const int outWidth = getMaxWidth(m_outputLabels);
    const int right = std::max(m_titleLabel->preferredSize(ctx).x(), inWidth + outWidth);

    const int totalHeight = std::ceilf(bottom * 1.0f / gridSpacing) * gridSpacing;
    const int totalWidth = std::ceilf(right * 1.0f / gridSpacing) * gridSpacing;
    return {totalWidth, totalHeight};
}

bool synui::DefaultUnitWidget::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) {

    if (Widget::mouseButtonEvent(p, button, down, modifiers))
        return true;

    Vector2i mousePos = p - position();

    // Check if title was clicked
    if (m_titleLabel && m_titleLabel->contains(mousePos)) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && down) {
            // Open textbox on dbl click
            if (modifiers & GLFW_MOD_DOUBLE_CLICK) {
                m_titleTextBox->setValue(m_titleLabel->caption());
                m_titleTextBox->setFixedSize({width(), m_titleLabel->height()});
                m_titleTextBox->setFontSize(m_titleLabel->fontSize()-2);
                m_titleTextBox->setVisible(true);
                m_titleTextBox->requestFocus();
                m_titleLabel->setVisible(false);
                screen()->performLayout();
                return true;
            }
        }

        // Delete unit on right click
        if (button == GLFW_MOUSE_BUTTON_RIGHT && down) {
            if (promptForDelete_()) {
                return true;
            }
        }
    } else if (m_titleTextBox && m_titleTextBox->visible()) {
        m_titleTextBox->callback()(m_titleTextBox->value());
        return true;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (down) {
            // Open unit editor if mouse click is not captured above.
            triggerEditorCallback();
            return false;
        }
    }

    return false;
}

synui::InputUnitWidget::InputUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : DefaultUnitWidget(a_parent, a_vm, a_unitId) {
    for (auto lbl : m_inputLabels) {
        removeChild(lbl.second);
    }
    m_inputLabels.clear();
}

synui::OutputUnitWidget::OutputUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : DefaultUnitWidget(a_parent, a_vm, a_unitId) {
    for (auto lbl : m_outputLabels) {
        removeChild(lbl.second);
    }
    m_outputLabels.clear();
}
