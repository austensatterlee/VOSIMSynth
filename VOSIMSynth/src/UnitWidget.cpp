#include "UnitWidget.h"
#include "CircuitWidget.h"
#include <Unit.h>
#include <VoiceManager.h>
#include <DSPMath.h>
#include "MainWindow.h"

synui::UnitWidget::UnitWidget(synui::CircuitWidget* a_parent, int a_unitId) :
    Widget(a_parent),
    m_parentCircuit(a_parent),
    m_drag(false),
    m_unitId(a_unitId)
{
    const syn::Unit& unit = m_parentCircuit->voiceManager()->getUnit(m_unitId);
    const syn::NamedContainer<syn::UnitPort, 8>& inputs = unit.inputs();
    const syn::NamedContainer<double, 8>& outputs = unit.outputs();

    int rowHeight = 0;
    while (rowHeight < 15) { rowHeight += m_parentCircuit->gridSpacing(); }

    ////
    // Setup grid layout
    std::vector<int> rowSizes(syn::MAX(inputs.size(), outputs.size()) + 1, rowHeight);
    std::vector<int> colSizes{ 2, 0 };
    // Special sizing for title label
    rowSizes[0] = rowHeight + m_parentCircuit->gridSpacing() * 0.5;
    // Create layout
    auto layout = new nanogui::AdvancedGridLayout(colSizes, rowSizes);
    layout->setColStretch(0, 1.0f);
    layout->setColStretch(1, 1.0f);
    using Anchor = nanogui::AdvancedGridLayout::Anchor;
    setLayout(layout);

    // Create title
    m_titleLabel = new nanogui::Label(this, unit.name(), "sans-bold", -1);
    m_titleLabel->setEnabled(false);
    layout->setAnchor(m_titleLabel, Anchor{ 0,0,2,1,nanogui::Alignment::Middle });

    // Create port labels	
    for (int i = 0; i < inputs.size(); i++)
    {
        int inputId = inputs.indices()[i];
        const string& inputName = inputs.name(inputId);
        auto lbl = new nanogui::Label(this, inputName, "sans", -1);
        layout->setAnchor(lbl, Anchor{ 0, inputs.size() - i });
        lbl->setId(std::to_string(inputId));
        m_inputLabels[inputId] = lbl;
    }
    for (int i = 0; i < outputs.size(); i++)
    {
        int outputId = outputs.indices()[i];
        const string& outputName = outputs.name(outputId);
        auto lbl = new nanogui::Label(this, outputName, "sans", -1);
        lbl->setTextAlign(nanogui::Label::Alignment::Right);
        layout->setAnchor(lbl, Anchor{ 1, outputs.size() - i });
        lbl->setId(std::to_string(outputId));
        m_outputLabels[outputId] = lbl;
    }
    // Fill empty rows with empty labels (for aesthetics)
    for(int i=outputs.size();i<rowSizes.size()-1;i++)
    {
        auto lbl = new nanogui::Label(this, "", "sans", -1);
        layout->setAnchor(lbl, Anchor{ 1, i+1 });
        m_emptyOutputLabels[i] = lbl;
    }
    for(int i=inputs.size();i<rowSizes.size()-1;i++)
    {
        auto lbl = new nanogui::Label(this, "", "sans", -1);
        layout->setAnchor(lbl, Anchor{ 1, i+1 });
        m_emptyInputLabels[i] = lbl;
    }
}

void synui::UnitWidget::draw(NVGcontext* ctx)
{
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
    auto drawDivisions = [&](const map<int, Widget*>& lbls) {
        for (auto lbl : lbls)
        {
            nvgBeginPath(ctx);
            if (i % 2 == 0)
                nvgFillColor(ctx, nanogui::Color(0.0f, 0.30f));
            else
                nvgFillColor(ctx, nanogui::Color(0.0f, 0.20f));
            nvgRect(ctx, lbl.second->position().x(), lbl.second->position().y(), lbl.second->width(), lbl.second->height());
            nvgFill(ctx);
            i++;
        }
    };
    drawDivisions(m_inputLabels);
    drawDivisions(m_emptyInputLabels);
    i=0;
    drawDivisions(m_outputLabels);
    drawDivisions(m_emptyOutputLabels);
    nvgRestore(ctx);

    Widget::draw(ctx);
}

bool synui::UnitWidget::mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers)
{
    Eigen::Vector2i mousePos = p - position();
    // Check if left mouse button was release while dragging
    if (m_drag && !down)
    {
        bool result = m_parentCircuit->updateUnitPos_(m_unitId, position());
        if (!result)
            setPosition(m_oldPos);
        m_drag = false;
        return true;
    }

    // Check if title was clicked
    if (m_titleLabel->contains(mousePos))
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (down)
            {
                m_oldPos = position();
                m_drag = true;
                return true;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (!down)
            {
                m_parentCircuit->deleteUnit_(m_unitId);
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
                    return true;
                }
                else
                {
                    m_parentCircuit->endWireDraw_(m_unitId, inputLabel.first, false);
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
                    return true;
                }
                else {
                    m_parentCircuit->endWireDraw_(m_unitId, outputLabel.first, true);
                    return true;
                }
            }
        }
    }
    return false;
}

bool synui::UnitWidget::mouseDragEvent(const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers)
{
    if (m_drag)
    {
        auto pcpy = p;
        pcpy = pcpy.cwiseMax(Eigen::Vector2i::Zero());
        pcpy = pcpy.cwiseMin(parent()->size() - mSize);
        setPosition(pcpy);
        return true;
    }
    return false;
}

Eigen::Vector2i synui::UnitWidget::getInputPortPosition(int a_portId)
{
    auto portPos = m_inputLabels[a_portId]->position();
    //portPos.y() += m_inputLabels[a_portId]->size().y();
    return portPos + position();
}

Eigen::Vector2i synui::UnitWidget::getOutputPortPosition(int a_portId)
{
    auto portPos = m_outputLabels[a_portId]->position();
    portPos.x() += m_outputLabels[a_portId]->size().x();
    //portPos.y() += m_outputLabels[a_portId]->size().y();
    return portPos + position();
}

Eigen::Vector2i synui::UnitWidget::preferredSize(NVGcontext* ctx) const
{
    auto pref_size = Widget::preferredSize(ctx);
    auto fixed_size = m_parentCircuit->fixToGrid(pref_size);
    fixed_size = (fixed_size.array() < pref_size.array()).select(fixed_size + Eigen::Vector2i::Ones() * m_parentCircuit->m_gridSpacing, fixed_size);
    return fixed_size;
}
