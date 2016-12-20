#include "UnitWidget.h"
#include "CircuitWidget.h"
#include <Unit.h>
#include <VoiceManager.h>
#include <DSPMath.h>
#include "MainWindow.h"
#include "MainGUI.h"

enum GenericEnum{};

synui::UnitWidget::UnitWidget(synui::CircuitWidget *a_parent, syn::VoiceManager *a_vm, int a_unitId) :
    Widget(a_parent),
    m_parentCircuit(a_parent),
    m_vm(a_vm),
    m_state(Uninitialized),
    m_unitId(a_unitId)
{
    const syn::Unit &unit = m_parentCircuit->voiceManager()->getUnit(m_unitId);
    const syn::NamedContainer<syn::UnitPort, 8>& inputs = unit.inputs();
    const syn::NamedContainer<double, 8>& outputs = unit.outputs();

    int rowHeight = 0;
    while (rowHeight < 15) { rowHeight += m_parentCircuit->gridSpacing(); }

    ////
    // Setup grid layout
    std::vector<int> rowSizes(syn::MAX(inputs.size(), outputs.size()) + 1, rowHeight);
    std::vector<int> colSizes{2, 0};
    // Special sizing for title label
    rowSizes[0] = rowHeight + m_parentCircuit->gridSpacing() * 0.5;
    // Create layout
    auto layout = new nanogui::AdvancedGridLayout(colSizes, rowSizes);
    layout->setColStretch(0, 1.0f);
    using Anchor = nanogui::AdvancedGridLayout::Anchor;
    setLayout(layout);

    // Create title
    m_titleLabel = new nanogui::Label(this, unit.name(), "sans-bold", -1);
    m_titleLabel->setEnabled(false);
    layout->setAnchor(m_titleLabel, Anchor{0,0,2,1,nanogui::Alignment::Middle});

    // Create port labels	
    for (int i = 0; i < inputs.size(); i++)
    {
        int inputId = inputs.indices()[i];
        const string &inputName = inputs.name(inputId);
        auto lbl = new nanogui::Label(this, inputName, "sans", -1);
        layout->setAnchor(lbl, Anchor{0, inputs.size() - i});
        lbl->setId(std::to_string(inputId));
        m_inputLabels[inputId] = lbl;
    }
    for (int i = 0; i < outputs.size(); i++)
    {
        int outputId = outputs.indices()[i];
        const string &outputName = outputs.name(outputId);
        auto lbl = new nanogui::Label(this, outputName, "sans", -1);
        lbl->setTextAlign(nanogui::Label::Alignment::Right);
        layout->setAnchor(lbl, Anchor{1, outputs.size() - i});
        lbl->setId(std::to_string(outputId));
        m_outputLabels[outputId] = lbl;
    }
    // Fill empty rows with empty labels (for aesthetics)
    for (int i = outputs.size(); i < rowSizes.size() - 1; i++)
    {
        auto lbl = new nanogui::Label(this, "", "sans", -1);
        layout->setAnchor(lbl, Anchor{1, i + 1});
        m_emptyOutputLabels[i] = lbl;
    }
    for (int i = inputs.size(); i < rowSizes.size() - 1; i++)
    {
        auto lbl = new nanogui::Label(this, "", "sans", -1);
        layout->setAnchor(lbl, Anchor{1, i + 1});
        m_emptyInputLabels[i] = lbl;
    }
}

void synui::UnitWidget::draw(NVGcontext *ctx)
{
    // Perform initialization that cannot be done in the constructor
    if(m_state==Uninitialized)
    {
        m_editorWidget = createEditor_(nullptr);
        m_editorWidget->setVisible(false);
        m_state = Idle;
    }

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
    auto drawDivisions = [&](const map<int, Widget*>& lbls, const nanogui::Color &c1, const nanogui::Color &c2)
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
    drawDivisions(m_inputLabels, nanogui::Color(0.0f, 0.30f), nanogui::Color(0.0f, 0.20f));
    drawDivisions(m_emptyInputLabels, nanogui::Color(0.0f, 0.30f), nanogui::Color(0.0f, 0.20f));
    i = 0;
    drawDivisions(m_outputLabels, nanogui::Color(0.1f, 0.1f, 0.0f, 0.30f), nanogui::Color(0.1f, 0.1f, 0.0f, 0.30f));
    drawDivisions(m_emptyOutputLabels, nanogui::Color(0.1f, 0.1f, 0.0f, 0.30f), nanogui::Color(0.1f, 0.1f, 0.0f, 0.30f));
    nvgRestore(ctx);

    Widget::draw(ctx);
}

bool synui::UnitWidget::mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers)
{
    Eigen::Vector2i mousePos = p - position();
    // Check if left mouse button was release while dragging
    if (m_state==Dragging && !down)
    {
        // If drag didn't move very far, interpret it as a normal click
        if((m_oldPos - position()).squaredNorm() < 1.0)
        { 
            if(m_callback)
                m_callback(m_editorWidget);
        }else{
            bool result = m_parentCircuit->updateUnitPos_(m_unitId, position());
            if (!result)
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
            if (down)
            {
                m_oldPos = position();
                m_state = Dragging;
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
                else
                {
                    m_parentCircuit->endWireDraw_(m_unitId, outputLabel.first, true);
                    return true;
                }
            }
        }
    }
    return false;
}

bool synui::UnitWidget::mouseDragEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers)
{
    if (m_state==Dragging)
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
    return portPos + position();
}

Eigen::Vector2i synui::UnitWidget::getOutputPortPosition(int a_portId)
{
    auto portPos = m_outputLabels[a_portId]->position();
    portPos.x() += m_outputLabels[a_portId]->size().x();
    return portPos + position();
}

Eigen::Vector2i synui::UnitWidget::preferredSize(NVGcontext *ctx) const
{
    auto pref_size = Widget::preferredSize(ctx);
    auto fixed_size = m_parentCircuit->fixToGrid(pref_size);
    fixed_size = (fixed_size.array() < pref_size.array()).select(fixed_size + Eigen::Vector2i::Ones() * m_parentCircuit->m_gridSpacing, fixed_size);
    return fixed_size;
}

nanogui::Widget *synui::UnitWidget::createEditor_(Widget *a_parent)
{
    Widget *screen = this;
    while (screen->parent())
        screen = screen->parent();

    nanogui::FormHelper *gui = new nanogui::FormHelper(static_cast<nanogui::Screen*>(screen));
    nanogui::Widget *editor = gui->addWindow();

    const syn::Unit &unit = m_parentCircuit->voiceManager()->getUnit(m_unitId);
    const int *paramIndices = unit.parameters().indices();
    int nParams = unit.numParams();
    gui->addGroup("Params");

    for (int pNum = 0; pNum < nParams; pNum++)
    {
        int paramId = paramIndices[pNum];
        const syn::UnitParameter &param = unit.param(paramId);
        syn::UnitParameter::EParamType paramType = param.getType();
        syn::UnitParameter::EControlType paramControlType = param.getControlType();
        const string &paramUnits = param.getUnitsString();
        switch (paramType)
        {
            case syn::UnitParameter::Null:
                continue;
                break;
            case syn::UnitParameter::Bool:
            {
                auto setter = [this, paramId](const bool &val)
                {
                    setParamValue(paramId, val);
                };
                auto getter = [this, paramId]()
                {
                    return m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).param(paramId).getBool();
                };
                nanogui::CheckBox *cb = gui->addVariable<bool>(unit.paramName(paramId), setter, getter, true);
                break;
            }
            case syn::UnitParameter::Enum:
            {
                auto setter = [this, paramId](const GenericEnum &val)
                {
                    setParamValue(paramId, val);
                };
                auto getter = [this, paramId]() -> GenericEnum { 
                    return static_cast<GenericEnum>(static_cast<int>(m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).param(paramId).getEnum())); 
                };
                nanogui::ComboBox *cb = gui->addVariable<GenericEnum>(unit.paramName(paramId), setter, getter, true);
                const std::vector<syn::UnitParameter::DisplayText>& options = param.getDisplayTexts();
                std::vector<string> items{options.size()};
                std::transform(options.begin(), options.end(), items.begin(), [](const syn::UnitParameter::DisplayText &dt){ return dt.m_text; });
                cb->setItems(items);
                break;
            }
            case syn::UnitParameter::Int:
            {
                auto setter = [this, paramId](const int &val)
                {
                    setParamValue(paramId, val);
                };
                auto getter = [this, paramId]()
                {
                    return m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).param(paramId).getInt();
                };
                nanogui::IntBox<int> *ib = gui->addVariable<int>(unit.paramName(paramId), setter, getter, true);
                ib->setMinValue(param.getMin());
                ib->setMaxValue(param.getMax());
                ib->setUnits(param.getUnitsString());
                ib->setSpinnable(true);
                break;
            }
            case syn::UnitParameter::Double:
            {
                auto setter = [this, paramId](const double &val)
                {
                    setParamValue(paramId, val);
                };
                auto getter = [this, paramId]()
                {
                    return m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).param(paramId).getDouble();
                };
                nanogui::FloatBox<double> *fb = gui->addVariable<double>(unit.paramName(paramId), setter, getter, true);
                fb->setMinValue(param.getMin());
                fb->setMaxValue(param.getMax());
                fb->setUnits(param.getUnitsString());
                fb->setValueIncrement(std::pow(10.0,-param.getPrecision()));
                fb->setSpinnable(true);
                break;
            }
            default:
                continue;
                break;
        }
    }
    return editor;
}

void synui::UnitWidget::setParamValue(int a_paramId, double a_val) const
{
    syn::RTMessage *msg = new syn::RTMessage();
    msg->action = [](syn::Circuit *a_circuit, bool a_isLast, ByteChunk *a_data)
            {
                double val;
                int unitId, paramId;
                GetArgs(a_data, 0, unitId, paramId, val);
                a_circuit->getUnit(unitId).param(paramId).set(val);
            };
    PutArgs(&msg->data, m_unitId, a_paramId, a_val);
    m_vm->queueAction(msg);
}

void synui::UnitWidget::setParamNorm(int a_paramId, double a_normval) const
{
    syn::RTMessage *msg = new syn::RTMessage();
    msg->action = [](syn::Circuit *a_circuit, bool a_isLast, ByteChunk *a_data)
            {
                double val;
                int unitId, paramId;
                int pos = 0;
                pos = a_data->Get<int>(&unitId, pos);
                pos = a_data->Get<int>(&paramId, pos);
                pos = a_data->Get<double>(&val, pos);
                a_circuit->getUnit(unitId).param(paramId).setNorm(val);
            };
    msg->data.Put<int>(&m_unitId);
    msg->data.Put<int>(&a_paramId);
    msg->data.Put<double>(&a_normval);
    m_vm->queueAction(msg);
}

void synui::UnitWidget::nudgeParam(int a_paramId, double a_logScale, double a_linScale) const
{
    syn::RTMessage *msg = new syn::RTMessage();
    PutArgs(&msg->data, m_unitId, a_paramId, a_logScale, a_linScale);
    msg->action = [](syn::Circuit *a_circuit, bool a_isLast, ByteChunk *a_data)
            {
                double logScale, linScale;
                int unitId, paramId;
                GetArgs(a_data, 0, unitId, paramId, logScale, linScale);
                a_circuit->getUnit(unitId).param(paramId).nudge(logScale, linScale);
            };
    m_vm->queueAction(msg);
}

void synui::UnitWidget::setParamFromString(int a_paramId, const string &a_str) const
{
    syn::RTMessage *msg = new syn::RTMessage();
    PutArgs(&msg->data, a_str, m_unitId, a_paramId);
    msg->action = [](syn::Circuit *a_circuit, bool a_isLast, ByteChunk *a_data)
            {
                string valStr;
                int unitId, paramId;
                GetArgs(a_data, 0, valStr, unitId, paramId);
                a_circuit->getUnit(unitId).param(paramId).setFromString(valStr);
            };
    m_vm->queueAction(msg);
}
