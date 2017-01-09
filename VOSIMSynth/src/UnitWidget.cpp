#include "UnitWidget.h"
#include "CircuitWidget.h"
#include <Unit.h>
#include <VoiceManager.h>
#include <DSPMath.h>
#include "MainWindow.h"
#include "MainGUI.h"

synui::UnitWidget::UnitWidget(synui::CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId) :
    Widget(a_parent),
    m_parentCircuit(a_parent),
    m_vm(a_vm),
    m_lastClickTime(0),
    m_state(Uninitialized),
    m_unitId(a_unitId),
    m_highlighted(false)
{
    setDraggable(false);

    const syn::Unit& unit = getUnit_();
    m_classIdentifier = unit.getClassIdentifier();
    setTooltip(unit.getClassName());

    const auto& inputs = unit.inputs();
    const auto& outputs = unit.outputs();

    ////
    // Setup grid layout
    std::vector<int> rowSizes(syn::MAX(inputs.size(), outputs.size()) + 1, 0);
    std::vector<int> colSizes{0, 5, 0};
    // Create layout
    auto layout = new nanogui::AdvancedGridLayout(colSizes, rowSizes);
    layout->setColStretch(1, 1.0f);
    using Anchor = nanogui::AdvancedGridLayout::Anchor;
    setLayout(layout);

    // Create title
    m_titleLabel = new nanogui::Label(this, unit.name(), "sans-bold", 0);
    m_titleLabel->setTooltip("Ctrl+click to edit title\nRight click to delete");
    m_titleLabel->setDraggable(false);
    m_titleLabel->setTextAlign(nanogui::Label::Alignment::Left);
    layout->setAnchor(m_titleLabel, Anchor{0,0,3,1,nanogui::Alignment::Middle});
    m_titleTextBox = new nanogui::TextBox(this, unit.name());
    m_titleTextBox->setEditable(true);
    m_titleTextBox->setVisible(false);
    m_titleTextBox->setCallback([this, layout](const std::string& a_str)
        {
            if (a_str.empty())
                return false;
            setName_(a_str);
            layout->removeAnchor(m_titleTextBox);
            layout->setAnchor(m_titleLabel, Anchor{0,0,3,1,nanogui::Alignment::Middle});
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
        layout->setAnchor(lbl, Anchor{0, inputs.size() - i});
        lbl->setId(std::to_string(inputId));
        lbl->setTooltip(inputName);
        lbl->setDraggable(false);
        m_inputLabels[inputId] = lbl;
    }
    // Create output port labels
    for (int i = 0; i < outputs.size(); i++)
    {
        int outputId = outputs.indices()[i];
        const string& outputName = outputs.name(outputId);
        auto lbl = new nanogui::Label(this, outputName, "sans", 0);
        lbl->setTextAlign(nanogui::Label::Alignment::Right);
        layout->setAnchor(lbl, Anchor{2, outputs.size() - i});
        lbl->setId(std::to_string(outputId));
        lbl->setTooltip(outputName);
        lbl->setDraggable(false);
        m_outputLabels[outputId] = lbl;
    }
    // Fill empty rows with empty labels (for aesthetics)
    for (int i = inputs.size(); i < rowSizes.size() - 1; i++)
    {
        auto lbl = new nanogui::Label(this, "", "sans", 0);
        layout->setAnchor(lbl, Anchor{0, i + 1});
        m_emptyInputLabels[i] = lbl;
    }
    for (int i = outputs.size(); i < rowSizes.size() - 1; i++)
    {
        auto lbl = new nanogui::Label(this, "", "sans", 0);
        layout->setAnchor(lbl, Anchor{2, i + 1});
        m_emptyOutputLabels[i] = lbl;
    }

    updateRowSizes_();
}

synui::UnitWidget::~UnitWidget() {}

void synui::UnitWidget::draw(NVGcontext* ctx)
{
    // Perform initialization that cannot be done in the constructor
    if (m_state == Uninitialized) { m_state = Idle; }
   
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

    nanogui::Color oColor(0.24f, 0.16f, 0.09f, 1.0f);
    nanogui::Color oColor2 = nanogui::Color(1.26f, 1.0f).cwiseProduct(oColor);
    nanogui::Color iColor(0.09f, 0.16f, 0.24f, 1.0f);
    nanogui::Color iColor2 = nanogui::Color(1.26f, 1.0f).cwiseProduct(iColor);
    drawDivisions(m_inputLabels, iColor, iColor2);
    //drawDivisions(m_emptyInputLabels, iColor, iColor2);
    i = 0;
    drawDivisions(m_outputLabels, oColor, oColor2);
    //drawDivisions(m_emptyOutputLabels, oColor, oColor2);

    /* Draw highlight if enabled. */
    if (highlighted())
    {
        drawShadow(ctx, 0, 0, width(), height(), 1.0f, 10.0f, 0.5f, {0.32f, 0.9f, 0.9f, 0.9f}, {0.0f, 0.0f});
    }

    /* Handle mouse movements */
    Vector2i mousePos = screen()->mousePos() - absolutePosition();
    if (contains(mousePos + position()))
    {
        // Highlight on mouse over
        drawShadow(ctx, 0, 0, width(), height(), 1.0f, 5.0f, 0.46f, {0.8f, 0.5f}, {0.0f, 0.0f});

        if (mousePos.y() < m_titleLabel->height())
        {
            // Highlight title
            nvgBeginPath(ctx);
            nvgRect(ctx, 0.0f, 0.0f, width(), m_titleLabel->height());
            nvgFillColor(ctx, nanogui::Color(1.0f, 0.05f));
            nvgFill(ctx);
        }
        else
        {
            // Highlight ports
            for (auto& p : m_inputLabels)
            {
                if (p.second->contains(mousePos))
                {
                    const Vector2i& pos = p.second->position();
                    const Vector2i& size = p.second->size();
                    nvgBeginPath(ctx);
                    nvgRect(ctx, pos.x(), pos.y(), size.x(), size.y());
                    nvgFillColor(ctx, nanogui::Color(1.0f, 0.05f));
                    nvgFill(ctx);
                    break;
                }
            }

            for (auto& p : m_outputLabels)
            {
                if (p.second->contains(mousePos))
                {
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

bool synui::UnitWidget::mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers)
{
    if(Widget::mouseButtonEvent(p, button, down, modifiers)) 
        return true;

    Eigen::Vector2i mousePos = p - position();
    double clickTime;
    bool dblClick = false;
    if (down)
    {
        clickTime = glfwGetTime() - m_lastClickTime;
        m_lastClickTime = glfwGetTime();
        dblClick = clickTime < 0.25;
    }

    // Check if title was clicked
    if (m_titleLabel->contains(mousePos))
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && down)
        {
            // Open textbox on ctrl+click
            if (dblClick)
            {
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
        if (button == GLFW_MOUSE_BUTTON_RIGHT && down)
        {
            // Delete unit on right click
            if (m_state == Idle)
            {
                if (getUnitId() != m_vm->getPrototypeCircuit()->getInputUnitId() && getUnitId() != m_vm->getPrototypeCircuit()->getOutputUnitId())
                {
                    auto dlg = new nanogui::MessageDialog(screen(), nanogui::MessageDialog::Type::Question, "Confirm deleteion", "Delete " + getName() + "?", "Delete", "Cancel", true);
                    dlg->setCallback([this](int x)
                        {
                            if (x == 0)
                            {
                                m_parentCircuit->deleteUnit_(m_unitId);
                            }
                        });
                    return true;
                }
            }
        }
    }
    else if (m_titleTextBox->visible())
    {
        m_titleTextBox->callback()(m_titleTextBox->value());
        return true;
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
                    return false;
                }
                else
                {
                    m_parentCircuit->endWireDraw_(m_unitId, inputLabel.first, false);
                    m_state = Idle;
                    return false;
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
                    return false;
                }
                else
                {
                    m_parentCircuit->endWireDraw_(m_unitId, outputLabel.first, true);
                    m_state = Idle;
                    return false;
                }
            }
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (down)
        {
            // Open unit editor if mouse click is not captured above.
            m_editorCallback(m_classIdentifier, m_unitId);
            return false;
        }
        else
        {
            m_state = Idle;
            return false;
        }
    }

    return false;
}

const std::string& synui::UnitWidget::getName() const
{
    return getUnit_().name();
}

Eigen::Vector2i synui::UnitWidget::getInputPortAbsPosition(int a_portId)
{
    auto& port = m_inputLabels[a_portId];
    return port->absolutePosition() + Vector2f{0, port->height() * 0.5}.cast<int>();
}

Eigen::Vector2i synui::UnitWidget::getOutputPortAbsPosition(int a_portId)
{
    auto& port = m_outputLabels[a_portId];
    return port->absolutePosition() + Vector2f{port->width(), port->height() * 0.5}.cast<int>();
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

    // Special sizing for title label (so grid point will be centered relative to port labels)
    int titleRowSize = rowHeight * 1.5;
    while (titleRowSize < 8)
        titleRowSize += rowHeight;
    int titleRowFontSize = titleRowSize > 12 ? titleRowSize - 2 : titleRowSize;
    l->setRowSize(0, titleRowSize);
    m_titleLabel->setFontSize(titleRowFontSize);
    int portRowSize = m_parentCircuit->getGridSpacing();
    int portRowFontSize = portRowSize < 10 ? 0 : portRowSize;

    // Apply new row size to port rows
    for (int r = 0; r < l->rowCount() - 1; r++)
    {
        if (m_inputLabels.find(r) != m_inputLabels.end())
        {
            m_inputLabels[r]->setFontSize(portRowFontSize);
            m_inputLabels[r]->setFixedWidth(portRowFontSize ? 0 : rowHeight);
            m_inputLabels[r]->setFixedHeight(portRowFontSize ? 0 : rowHeight);
        }
        if (m_outputLabels.find(r) != m_outputLabels.end())
        {
            m_outputLabels[r]->setFontSize(portRowFontSize);
            m_outputLabels[r]->setFixedWidth(portRowFontSize ? 0 : rowHeight);
            m_outputLabels[r]->setFixedHeight(portRowFontSize ? 0 : rowHeight);
        }
        l->setRowSize(r + 1, rowHeight);
    }

    setSize({0,0});
}

const syn::Unit& synui::UnitWidget::getUnit_() const { return m_vm->getUnit(m_unitId); }

void synui::UnitWidget::setName_(const std::string& a_name)
{
    m_titleLabel->setCaption(a_name);
    m_vm->getUnit(m_unitId).setName(a_name);
}
