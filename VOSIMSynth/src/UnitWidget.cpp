#include "UnitWidget.h"
#include "CircuitWidget.h"
#include <Unit.h>
#include <VoiceManager.h>
#include <DSPMath.h>
#include "MainWindow.h"

synui::UnitWidget::UnitWidget(synui::CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId) :
    Widget(a_parent),
    m_parentCircuit(a_parent),
    m_vm(a_vm),
    m_state(Uninitialized),
    m_unitId(a_unitId),
    m_highlighted(false)
{
    const syn::Unit& unit = getUnit_();
    m_classIdentifier = unit.getClassIdentifier();
    setTooltip(unit.getClassName());

    const auto& inputs = unit.inputs();
    const auto& outputs = unit.outputs();

    ////
    // Setup grid layout
    std::vector<int> rowSizes(syn::MAX(inputs.size(), outputs.size()) + 1, 0);
    std::vector<int> colSizes{ 0, 5, 0 };
    // Create layout
    auto layout = new nanogui::AdvancedGridLayout(colSizes, rowSizes);
    layout->setColStretch(1, 1.0f);
    using Anchor = nanogui::AdvancedGridLayout::Anchor;
    setLayout(layout);

    // Create title
    m_titleLabel = new nanogui::Label(this, unit.name(), "sans-bold", 0);
    m_titleLabel->setTooltip("Ctrl+click to edit");
    m_titleLabel->setDraggable(false);
    layout->setAnchor(m_titleLabel, Anchor{ 0,0,3,1,nanogui::Alignment::Middle });
    m_titleTextBox = new nanogui::TextBox(m_parentCircuit, unit.name());
    m_titleTextBox->setEditable(true);
    m_titleTextBox->setVisible(false);
    m_titleTextBox->setCallback([this](const std::string& a_str)
    {
        if (a_str.empty()) return false;
        setName_(a_str);
        m_titleLabel->setVisible(true);
        m_titleTextBox->setVisible(false);
        updateRowSizes_();
        screen()->performLayout();
        m_parentCircuit->updateUnitPos_(this, position(), true);
        return true;
    });

    // Create input port labels
    for (int i = 0; i < inputs.size(); i++)
    {
        int inputId = inputs.indices()[i];
        const string& inputName = inputs.name(inputId);
        auto lbl = new nanogui::Label(this, inputName, "sans", 0);
        layout->setAnchor(lbl, Anchor{ 0, inputs.size() - i });
        lbl->setId(std::to_string(inputId));
        lbl->setTooltip(inputName);
        m_inputLabels[inputId] = lbl;
    }
    // Create output port labels
    for (int i = 0; i < outputs.size(); i++)
    {
        int outputId = outputs.indices()[i];
        const string& outputName = outputs.name(outputId);
        auto lbl = new nanogui::Label(this, outputName, "sans", 0);
        lbl->setTextAlign(nanogui::Label::Alignment::Right);
        layout->setAnchor(lbl, Anchor{ 2, outputs.size() - i });
        lbl->setId(std::to_string(outputId));
        lbl->setTooltip(outputName);
        m_outputLabels[outputId] = lbl;
    }
    // Fill empty rows with empty labels (for aesthetics)
    for (int i = inputs.size(); i < rowSizes.size() - 1; i++)
    {
        auto lbl = new nanogui::Label(this, "", "sans", 0);
        layout->setAnchor(lbl, Anchor{ 0, i + 1 });
        m_emptyInputLabels[i] = lbl;
    }
    for (int i = outputs.size(); i < rowSizes.size() - 1; i++)
    {
        auto lbl = new nanogui::Label(this, "", "sans", 0);
        layout->setAnchor(lbl, Anchor{ 2, i + 1 });
        m_emptyOutputLabels[i] = lbl;
    }

    updateRowSizes_();
}

void synui::UnitWidget::draw(NVGcontext* ctx)
{
    // Perform initialization that cannot be done in the constructor
    if (m_state == Uninitialized) { m_state = Idle; }

    nvgSave(ctx);
    nvgTranslate(ctx, mPos.x(), mPos.y());
    // draw background
    nvgFillColor(ctx, nanogui::Color(0.3f, 0.9f));
    nvgStrokeColor(ctx, nanogui::Color(0.0f, 0.9f));
    nvgStrokeWidth(ctx, 1.0f);
    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, mSize.x(), mSize.y());
    nvgFill(ctx);
    nvgStroke(ctx);

    // draw divisions
    int i = 0;
    auto drawDivisions = [&](const map<int, Widget*>& lbls, const nanogui::Color& c1, const nanogui::Color& c2)
    {
        for (auto lbl : lbls)
        {
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

    nanogui::Color oColor(0.1f, 0.07f, 0.0f, 0.3f);
    nanogui::Color oColor2 = Vector4f(1, 1, 1, 0.667).cwiseProduct(oColor);
    nanogui::Color iColor(0.0f, 0.3f);
    nanogui::Color iColor2 = Vector4f(1, 1, 1, 0.667).cwiseProduct(iColor);
    drawDivisions(m_inputLabels, iColor, iColor2);
    //drawDivisions(m_emptyInputLabels, iColor, iColor2);
    i = 0;
    drawDivisions(m_outputLabels, oColor, oColor2);
    //drawDivisions(m_emptyOutputLabels, oColor, oColor2);

    // Handle mouse movements
    Vector2i mousePos = screen()->mousePos() - absolutePosition();
    if (m_state == BodyClicked)
    {
        double distanceCovered = (mousePos - m_clickPos).squaredNorm();
        if (distanceCovered > 1.0) { m_state = BodyDragging; }
    }

    if (m_titleLabel->contains(mousePos))
    {
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, 0, 0, width(), m_titleLabel->height(), 2.0f);
        nvgFillColor(ctx, nanogui::Color(1.0f, 0.125f));
        nvgFill(ctx);
    }
    if (contains(mousePos+position()))
    {
        drawShadow(ctx, 0, 0, width(), height(), 2.0f, 5.0f, { 0.8f, 0.5f }, { 0.0f, 0.0f });
    }

    if (highlighted())
    {
        drawShadow(ctx, 0, 0, width(), height(), 1.0f, 6.0f, { 1.0f, 0.5f }, { 0.0f, 0.0f });
    }
    nvgRestore(ctx);

    Widget::draw(ctx);
}

bool synui::UnitWidget::mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers)
{
    Widget::mouseButtonEvent(p, button, down, modifiers);

    Eigen::Vector2i mousePos = p - position();

    if (m_state == BodyDragging)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && !down)
        {
            m_state = Idle;
            // Finalize position by notifying parent circuit
            if (m_parentCircuit->checkUnitPos_(this, position()))
                m_parentCircuit->updateUnitPos_(this, position());
            else
                setPosition(m_oldPos);
            m_state = Idle;
            return true;
        }
    }

    // Check if title was clicked
    if (m_titleLabel->contains(mousePos))
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            // Open textbox on ctrl+click
            if (modifiers & GLFW_MOD_CONTROL) {
                m_titleTextBox->setValue(m_titleLabel->caption());
                m_titleTextBox->setPosition(position());
                m_titleTextBox->setSize({ width(), m_titleLabel->height() });
                m_titleTextBox->setFontSize(m_titleLabel->fontSize());
                m_titleTextBox->setVisible(true);
                m_titleTextBox->setEditable(true);
                m_titleLabel->setVisible(false);
                return true;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            // Delete unit on right click
            if (!down)
            {
                m_parentCircuit->deleteUnit_(m_unitId);
                m_state = Idle;
                return true;
            }
        }
    }

    // Check if an input port was clicked
    for (auto inputLabel : m_inputLabels)
    {
        if (inputLabel.second->contains(mousePos))
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
                if (down)
                {
                    m_parentCircuit->startWireDraw_(m_unitId, inputLabel.first, false);
                    m_state = Idle;
                    return true;
                }
                else
                {
                    m_parentCircuit->endWireDraw_(m_unitId, inputLabel.first, false);
                    m_state = Idle;
                    return true;
                }
            }
        }
    }

    // Check if an output port was clicked
    for (auto outputLabel : m_outputLabels)
    {
        if (outputLabel.second->contains(mousePos))
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
                if (down)
                {
                    m_parentCircuit->startWireDraw_(m_unitId, outputLabel.first, true);
                    m_state = Idle;
                    return true;
                }
                else
                {
                    m_parentCircuit->endWireDraw_(m_unitId, outputLabel.first, true);
                    m_state = Idle;
                    return true;
                }
            }
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (down) {
            m_state = BodyClicked;
            m_oldPos = position();
            m_clickPos = mousePos;
            return true;
        }
        else
        {
            // Open unit editor if mouse click is not captured above.
            m_editorCallback(m_classIdentifier, m_unitId);
            return true;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && !down) {
      
    }

    return false;
}

bool synui::UnitWidget::mouseDragEvent(const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers)
{
    if (m_state == BodyDragging)
    {
        Eigen::Vector2i newPos = p - m_clickPos;
        newPos = newPos.cwiseMax(Eigen::Vector2i::Zero());
        newPos = newPos.cwiseMin(parent()->size() - mSize);
        setPosition(newPos);
        return true;
    }
    return false;
}

const std::string& synui::UnitWidget::getName() const {
    return getUnit_().name();
}

Eigen::Vector2i synui::UnitWidget::getInputPortAbsPosition(int a_portId)
{
    auto& port = m_inputLabels[a_portId];
    return port->absolutePosition() + Vector2f{ 0, m_parentCircuit->getGridSpacing() * 0.5 }.cast<int>();
}

Eigen::Vector2i synui::UnitWidget::getOutputPortAbsPosition(int a_portId)
{
    auto& port = m_outputLabels[a_portId];
    return port->absolutePosition() + Vector2f{ port->width(), m_parentCircuit->getGridSpacing() * 0.5 }.cast<int>();
}

Eigen::Vector2i synui::UnitWidget::preferredSize(NVGcontext* ctx) const
{
    // Snap size to grid
    Vector2f pref_size = Widget::preferredSize(ctx).cast<float>();
    Vector2f fixed_size = (pref_size / m_parentCircuit->getGridSpacing()).unaryExpr([](float a) { return ceil(a); });
    fixed_size *= m_parentCircuit->getGridSpacing();
    return fixed_size.cast<int>();
}

void synui::UnitWidget::performLayout(NVGcontext* ctx)
{
    Widget::performLayout(ctx);
}

synui::UnitWidget::operator json() const
{
    json j;
    j["x"] = position().x();
    j["y"] = position().y();
    return j;
}
synui::UnitWidget* synui::UnitWidget::load(const json& j)
{
    return this;
}

void synui::UnitWidget::updateRowSizes_()
{
    auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());

    int rowHeight = m_parentCircuit->getGridSpacing();
    while (rowHeight < 10) { rowHeight += m_parentCircuit->getGridSpacing(); }

    // Special sizing for title label (so grid point will be centered relative to port labels)
    int titleRowSize = rowHeight + m_parentCircuit->getGridSpacing() * 0.5;
    int titleRowFontSize = titleRowSize;
    l->setRowSize(0, titleRowSize);
    m_titleLabel->setFontSize(titleRowFontSize);
    int portRowSize = m_parentCircuit->getGridSpacing();
    int portRowFontSize = portRowSize < 10 ? 0 : portRowSize;

    // Apply new row size to port rows
    for (int r = 0; r < l->rowCount() - 1; r++)
    {
        if (m_inputLabels.find(r) != m_inputLabels.end())
            m_inputLabels[r]->setFontSize(portRowFontSize);
        if (m_outputLabels.find(r) != m_outputLabels.end())
            m_outputLabels[r]->setFontSize(portRowFontSize);
        l->setRowSize(r + 1, rowHeight);
    }

    setSize({ 0,0 });
}

const syn::Unit& synui::UnitWidget::getUnit_() const { return m_vm->getUnit(m_unitId); }

void synui::UnitWidget::setName_(const std::string& a_name)
{
    m_titleLabel->setCaption(a_name);
    m_vm->getUnit(m_unitId).setName(a_name);
}