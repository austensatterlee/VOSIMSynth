#include "DefaultUnitWidget.h"
#include "CircuitWidget.h"
#include "UI.h"
#include <DSPMath.h>
#include <Unit.h>
#include <vector>
#include <string>

synui::DefaultUnitWidget::DefaultUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : UnitWidget(a_parent, a_vm, a_unitId),
      m_lastClickTime(0) {
    const syn::Unit& unit = getUnit_();

    const auto& inputs = unit.inputs();
    const auto& outputs = unit.outputs();

    ////
    // Setup grid layout
    vector<int> rowSizes(syn::MAX(inputs.size(), outputs.size()) + 1, 0);
    vector<int> colSizes{0, 0, 0};
    // Create layout
    auto layout = new nanogui::AdvancedGridLayout(colSizes, rowSizes);
    layout->setColStretch(1, 1.0f);
    using Anchor = nanogui::AdvancedGridLayout::Anchor;
    setLayout(layout);

    // Create title
    m_titleLabel = new nanogui::Label(this, "", "sans-bold", 0);
    m_titleLabel->setTooltip("Dbl click to rename\nRight click to delete");
    m_titleLabel->setDraggable(false);
    m_titleLabel->setTextAlign(nanogui::Label::Alignment::Left);
    layout->setAnchor(m_titleLabel, Anchor{0,0,3,1,nanogui::Alignment::Middle});
    m_titleTextBox = new nanogui::TextBox(this, "");
    m_titleTextBox->setEditable(true);
    m_titleTextBox->setVisible(false);
    m_titleTextBox->setCallback([this, layout](const string& a_str) {
            if (a_str.empty())
                return false;
            setName(a_str);
            layout->removeAnchor(m_titleTextBox);
            layout->setAnchor(m_titleLabel, Anchor{0,0,3,1,nanogui::Alignment::Middle});
            m_titleLabel->setVisible(true);
            m_titleTextBox->setVisible(false);
            onGridChange_();
            screen()->performLayout();
            m_parentCircuit->updateUnitPos(this, position(), true);
            return true;
        });

    // Create input port labels
    for (int i = 0; i < inputs.size(); i++) {
        int inputId = inputs.ids()[i];
        const string& inputName = inputs.getNameFromId(inputId);
        auto lbl = new nanogui::Label(this, inputName, "sans", 0);
        layout->setAnchor(lbl, Anchor{0, 1 + i});
        lbl->setId(std::to_string(inputId));
        lbl->setTooltip(inputName);
        lbl->setDraggable(false);
        m_inputLabels[inputId] = lbl;
    }
    // Create output port labels
    for (int i = 0; i < outputs.size(); i++) {
        int outputId = outputs.ids()[i];
        const string& outputName = outputs.getNameFromId(outputId);
        auto lbl = new nanogui::Label(this, outputName, "sans", 0);
        layout->setAnchor(lbl, Anchor{2, 1 + i});
        lbl->setId(std::to_string(outputId));
        lbl->setTooltip(outputName);
        lbl->setDraggable(false);
        m_outputLabels[outputId] = lbl;
    }
    // Fill empty rows with empty labels (for aesthetics)
    for (int i = inputs.size(); i < rowSizes.size() - 1; i++) {
        auto lbl = new nanogui::Label(this, "", "sans", 0);
        layout->setAnchor(lbl, Anchor{0, i + 1});
        m_emptyInputLabels[i] = lbl;
    }
    for (int i = outputs.size(); i < rowSizes.size() - 1; i++) {
        auto lbl = new nanogui::Label(this, "", "sans", 0);
        layout->setAnchor(lbl, Anchor{2, i + 1});
        m_emptyOutputLabels[i] = lbl;
    }

    DefaultUnitWidget::onGridChange_();
}

void synui::DefaultUnitWidget::draw(NVGcontext* ctx) {
    m_titleLabel->setCaption(getName());

    nvgSave(ctx);
    nvgTranslate(ctx, mPos.x(), mPos.y());

    // Draw background
    nanogui::Color bgColor = nanogui::Color(0.3f, 0.9f);
    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, mSize.x(), mSize.y());
    nvgFillColor(ctx, bgColor);
    nvgFill(ctx);

    // Draw title background
    nanogui::Color titleBgColor = nanogui::Color(0.325f, 1.0f);
    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, width(), m_titleLabel->height() - 1.5f);
    nvgFillColor(ctx, titleBgColor);
    nvgFill(ctx);

    // Draw divisions
    int i = 0;
    auto drawDivisions = [&](const map<int, nanogui::Label*>& lbls, const nanogui::Color& c1, const nanogui::Color& c2) {
                for (auto lbl : lbls) {
                    nvgBeginPath(ctx);
                    if (i % 2 == 0)
                        nvgFillColor(ctx, c1);
                    else
                        nvgFillColor(ctx, c2);
                    nvgRect(ctx, lbl.second->position().x(), lbl.second->position().y(), lbl.second->width(), lbl.second->height());
                    nvgFill(ctx);
                    i++;
                }
            };

    nanogui::Color oColor(0.24f, 0.16f, 0.09f, 1.0f);
    nanogui::Color oColor2 = nanogui::Color(1.26f, 1.0f).cwiseProduct(oColor);
    nanogui::Color iColor(0.09f, 0.16f, 0.24f, 1.0f);
    nanogui::Color iColor2 = nanogui::Color(1.26f, 1.0f).cwiseProduct(iColor);
    drawDivisions(m_inputLabels, iColor, iColor2);
    i = 0;
    drawDivisions(m_outputLabels, oColor, oColor2);

    /* Draw highlight if enabled. */
    if (highlighted()) {
        drawRectShadow(ctx, 0, 0, width(), height(), 1.0f, 15.0f, 0.5f, {0.32f, 0.9f, 0.9f, 0.9f}, {0.0f, 0.0f});
    }

    /* Handle mouse movements */
    Vector2i mousePos = screen()->mousePos() - absolutePosition();
    if (contains(mousePos + position())) {
        // Highlight on mouse over
        drawRectShadow(ctx, 0, 0, width(), height(), 1.0f, 5.0f, 0.46f, {0.8f, 0.5f}, {0.0f, 0.0f});

        if (mousePos.y() < m_titleLabel->height()) {
            // Highlight title
            nvgBeginPath(ctx);
            nvgRect(ctx, 0.0f, 0.0f, width(), m_titleLabel->height());
            nvgFillColor(ctx, nanogui::Color(1.0f, 0.05f));
            nvgFill(ctx);
        } else {
            // Highlight ports
            for (auto& p : m_inputLabels) {
                if (p.second->contains(mousePos)) {
                    const Vector2i& pos = p.second->position();
                    const Vector2i& size = p.second->size();
                    nvgBeginPath(ctx);
                    nvgRect(ctx, pos.x(), pos.y(), size.x(), size.y());
                    nvgFillColor(ctx, nanogui::Color(1.0f, 0.05f));
                    nvgFill(ctx);
                    break;
                }
            }

            for (auto& p : m_outputLabels) {
                if (p.second->contains(mousePos)) {
                    const Vector2i& pos = p.second->position();
                    const Vector2i& size = p.second->size();
                    nvgBeginPath(ctx);
                    nvgRect(ctx, pos.x(), pos.y(), size.x(), size.y());
                    nvgFillColor(ctx, nanogui::Color(1.0f, 0.05f));
                    nvgFill(ctx);
                    break;
                }
            }
        }
    }

    // Draw outline
    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, mSize.x(), mSize.y());
    nvgStrokeColor(ctx, nanogui::Color(0.0f, 0.9f));
    nvgStrokeWidth(ctx, 1.0f);
    nvgStroke(ctx);

    nvgRestore(ctx);

    Widget::draw(ctx);
}

void synui::DefaultUnitWidget::setName(const string& a_name) {
    UnitWidget::setName(a_name);
    m_titleLabel->setCaption(a_name);
    m_titleTextBox->setValue(a_name);
}

Eigen::Vector2i synui::DefaultUnitWidget::getInputPortAbsPosition(int a_portId) {
    auto& port = m_inputLabels[a_portId];
    return port->absolutePosition() + Vector2f{0, port->height() * 0.5}.cast<int>();
}

Eigen::Vector2i synui::DefaultUnitWidget::getOutputPortAbsPosition(int a_portId) {
    auto& port = m_outputLabels[a_portId];
    return port->absolutePosition() + Vector2f{port->width(), port->height() * 0.5}.cast<int>();
}

int synui::DefaultUnitWidget::getInputPort(const Eigen::Vector2i& a_pos) {    
    int i = 0;
    for (auto inputLabel : m_inputLabels) {
        if (inputLabel.second->contains(a_pos)) {
            return i;
        }
        i++;
    }
    return -1;
}

int synui::DefaultUnitWidget::getOutputPort(const Eigen::Vector2i& a_pos) {
    int i = 0;
    for (auto outputLabel : m_outputLabels) {
        if (outputLabel.second->contains(a_pos)) {
            return i;
        }
        i++;
    }
    return -1;
}

Eigen::Vector2i synui::DefaultUnitWidget::preferredSize(NVGcontext* ctx) const {
    // Snap size to grid
    Vector2f pref_size = Widget::preferredSize(ctx).cast<float>();
    Vector2f fixed_size = (pref_size / m_parentCircuit->gridSpacing()).unaryExpr([](float a) { return ceil(a); });
    fixed_size *= m_parentCircuit->gridSpacing();
    return fixed_size.cast<int>();
}

bool synui::DefaultUnitWidget::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) {

    if (Widget::mouseButtonEvent(p, button, down, modifiers))
        return true;

    Vector2i mousePos = p - position();
    bool dblClick = false;
    if (down) {
        double clickTime = glfwGetTime() - m_lastClickTime;
        m_lastClickTime = glfwGetTime();
        dblClick = clickTime < 0.25;
    }

    // Check if title was clicked
    if (m_titleLabel && m_titleLabel->contains(mousePos)) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && down) {
            // Open textbox on ctrl+click
            if (dblClick) {
                auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());
                l->removeAnchor(m_titleLabel);
                l->setAnchor(m_titleTextBox, nanogui::AdvancedGridLayout::Anchor{0,0,3,1,nanogui::Alignment::Fill});
                m_titleTextBox->setValue(m_titleLabel->caption());
                m_titleTextBox->setPosition(position());
                m_titleTextBox->setFixedSize({width(), m_titleLabel->height()});
                m_titleTextBox->setFontSize(m_titleLabel->fontSize());
                m_titleTextBox->setVisible(true);
                m_titleTextBox->setEditable(true);
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
            return true;
        }
    }

    return true;
}

void synui::DefaultUnitWidget::onGridChange_() {
    auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());

    int rowHeight = m_parentCircuit->gridSpacing();

    // Special sizing for title label (so grid point will be centered relative to port labels)
    if (m_titleLabel) {
        int titleRowSize = rowHeight * 1.5;
        while (titleRowSize < 8)
            titleRowSize += rowHeight;
        int titleRowFontSize = titleRowSize > 12 ? titleRowSize - 2 : titleRowSize;
        l->setRowSize(0, titleRowSize);
        m_titleLabel->setFontSize(titleRowFontSize);
    }
    int portRowSize = m_parentCircuit->gridSpacing();
    int portRowFontSize = portRowSize;


    const syn::Unit& unit = getUnit_();
    const auto& inputs = unit.inputs();
    const auto& outputs = unit.outputs();
    // Apply new row size to port rows
    for (int r = 0; r < l->rowCount() - 1; r++) {
        if (m_inputLabels.find(r) != m_inputLabels.end()) {
            m_inputLabels[r]->setFontSize(portRowFontSize);
            if (portRowFontSize < 12) {
                m_inputLabels[r]->setCaption(inputs.getNameFromId(r));
                m_inputLabels[r]->setFixedWidth(rowHeight);
                m_inputLabels[r]->setFixedHeight(rowHeight);
            } else {
                m_inputLabels[r]->setCaption(inputs.getNameFromId(r));
                m_inputLabels[r]->setFixedWidth(0);
                m_inputLabels[r]->setFixedHeight(0);
            }
        }
        if (m_outputLabels.find(r) != m_outputLabels.end()) {
            m_outputLabels[r]->setFontSize(portRowFontSize);
            if (portRowFontSize < 12) {
                m_outputLabels[r]->setCaption(outputs.getNameFromId(r).substr(0, 2));
                m_outputLabels[r]->setFixedWidth(rowHeight);
                m_outputLabels[r]->setFixedHeight(rowHeight);
            } else {
                m_outputLabels[r]->setCaption(outputs.getNameFromId(r));
                m_outputLabels[r]->setFixedWidth(0);
                m_outputLabels[r]->setFixedHeight(0);
            }
        }
        l->setRowSize(r + 1, rowHeight);
    }
}

synui::InputUnitWidget::InputUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : DefaultUnitWidget(a_parent, a_vm, a_unitId) {
    for (auto lbl : m_inputLabels) {
        auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());
        l->removeAnchor(lbl.second);
        removeChild(lbl.second);
    }
    m_inputLabels.clear();
}

synui::OutputUnitWidget::OutputUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : DefaultUnitWidget(a_parent, a_vm, a_unitId) {
    for (auto lbl : m_outputLabels) {
        auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());
        l->removeAnchor(lbl.second);
        removeChild(lbl.second);
    }
    m_outputLabels.clear();
}
