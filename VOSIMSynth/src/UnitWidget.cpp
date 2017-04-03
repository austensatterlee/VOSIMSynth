#include "UnitWidget.h"
#include "CircuitWidget.h"
#include <Unit.h>
#include <VoiceManager.h>
#include <DSPMath.h>
#include "MainWindow.h"
#include "MainGUI.h"

synui::UnitWidget::UnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId) :
    Widget(a_parent),
    m_parentCircuit(a_parent),
    m_vm(a_vm),
    m_unitId(a_unitId),
    m_classIdentifier(getUnit_().getClassIdentifier()),
    m_highlighted(false)
{
    setDraggable(false);
    setTooltip(getUnit_().getClassName());
}

synui::UnitWidget::~UnitWidget() {}

const string& synui::UnitWidget::getName() const
{
    return getUnit_().name();
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

const syn::Unit& synui::UnitWidget::getUnit_() const { return m_vm->getUnit(m_unitId); }

void synui::UnitWidget::setName_(const string& a_name)
{
    m_vm->getUnit(m_unitId).setName(a_name);
}

bool synui::UnitWidget::promptForDelete_()
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
    return false;
}


void synui::UnitWidget::triggerPortDrag_(int a_portId, bool a_isOutput)
{
    m_parentCircuit->startWireDraw_(m_unitId, a_portId, a_isOutput);
}
void synui::UnitWidget::triggerPortDrop_(int a_portId, bool a_isOutput)
{
    m_parentCircuit->endWireDraw_(m_unitId, a_portId, a_isOutput);
}

synui::DefaultUnitWidget::DefaultUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : UnitWidget(a_parent, a_vm, a_unitId), m_lastClickTime(0)
{
    const syn::Unit& unit = getUnit_();

    const auto& inputs = unit.inputs();
    const auto& outputs = unit.outputs();

    ////
    // Setup grid layout
    vector<int> rowSizes(syn::MAX(inputs.size(), outputs.size()) + 1, 0);
    vector<int> colSizes{ 0, 5, 0 };
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
    layout->setAnchor(m_titleLabel, Anchor{ 0,0,3,1,nanogui::Alignment::Middle });
    m_titleTextBox = new nanogui::TextBox(this, unit.name());
    m_titleTextBox->setEditable(true);
    m_titleTextBox->setVisible(false);
    m_titleTextBox->setCallback([this, layout](const string& a_str)
    {
        if (a_str.empty())
            return false;
        setName_(a_str);
        layout->removeAnchor(m_titleTextBox);
        layout->setAnchor(m_titleLabel, Anchor{ 0,0,3,1,nanogui::Alignment::Middle });
        m_titleLabel->setVisible(true);
        m_titleTextBox->setVisible(false);
        onGridChange_();
        screen()->performLayout();
        m_parentCircuit->updateUnitPos(this, position(), true);
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
        lbl->setDraggable(false);
        m_inputLabels[inputId] = lbl;
    }
    // Create output port labels
    for (int i = 0; i < outputs.size(); i++)
    {
        int outputId = outputs.indices()[i];
        const string& outputName = outputs.name(outputId);
        auto lbl = new nanogui::Label(this, outputName, "sans", 0);
        //lbl->setTextAlign(nanogui::Label::Alignment::Right);
        layout->setAnchor(lbl, Anchor{ 2, outputs.size() - i });
        lbl->setId(std::to_string(outputId));
        lbl->setTooltip(outputName);
        lbl->setDraggable(false);
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

    DefaultUnitWidget::onGridChange_();
}

void synui::DefaultUnitWidget::draw(NVGcontext* ctx)
{
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
    auto drawDivisions = [&](const map<int, nanogui::Label*>& lbls, const nanogui::Color& c1, const nanogui::Color& c2)
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
    i = 0;
    drawDivisions(m_outputLabels, oColor, oColor2);

    /* Draw highlight if enabled. */
    if (highlighted())
    {
        drawShadow(ctx, 0, 0, width(), height(), 1.0f, 10.0f, 0.5f, { 0.32f, 0.9f, 0.9f, 0.9f }, { 0.0f, 0.0f });
    }

    /* Handle mouse movements */
    Vector2i mousePos = screen()->mousePos() - absolutePosition();
    if (contains(mousePos + position()))
    {
        // Highlight on mouse over
        drawShadow(ctx, 0, 0, width(), height(), 1.0f, 5.0f, 0.46f, { 0.8f, 0.5f }, { 0.0f, 0.0f });

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

Eigen::Vector2i synui::DefaultUnitWidget::getInputPortAbsPosition(int a_portId) {
    auto& port = m_inputLabels[a_portId];
    return port->absolutePosition() + Vector2f{ 0, port->height() * 0.5 }.cast<int>();
}

Eigen::Vector2i synui::DefaultUnitWidget::getOutputPortAbsPosition(int a_portId) {
    auto& port = m_outputLabels[a_portId];
    return port->absolutePosition() + Vector2f{ port->width(), port->height() * 0.5 }.cast<int>();
}

Eigen::Vector2i synui::DefaultUnitWidget::preferredSize(NVGcontext* ctx) const {
    // Snap size to grid
    Vector2f pref_size = Widget::preferredSize(ctx).cast<float>();
    Vector2f fixed_size = (pref_size / m_parentCircuit->getGridSpacing()).unaryExpr([](float a) { return ceil(a); });
    fixed_size *= m_parentCircuit->getGridSpacing();
    return fixed_size.cast<int>();
}

bool synui::DefaultUnitWidget::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers)
{

    if (Widget::mouseButtonEvent(p, button, down, modifiers))
        return true;

    Vector2i mousePos = p - position();
    double clickTime;
    bool dblClick = false;
    if (down)
    {
        clickTime = glfwGetTime() - m_lastClickTime;
        m_lastClickTime = glfwGetTime();
        dblClick = clickTime < 0.25;
    }

    // Check if title was clicked
    if (m_titleLabel && m_titleLabel->contains(mousePos))
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && down)
        {
            // Open textbox on ctrl+click
            if (dblClick)
            {
                auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());
                l->removeAnchor(m_titleLabel);
                l->setAnchor(m_titleTextBox, nanogui::AdvancedGridLayout::Anchor{ 0,0,3,1,nanogui::Alignment::Fill });
                m_titleTextBox->setValue(m_titleLabel->caption());
                m_titleTextBox->setPosition(position());
                m_titleTextBox->setFixedSize({ width(), m_titleLabel->height() });
                m_titleTextBox->setFontSize(m_titleLabel->fontSize());
                m_titleTextBox->setVisible(true);
                m_titleTextBox->setEditable(true);
                m_titleLabel->setVisible(false);
                screen()->performLayout();
                return true;
            }
        }

        // Delete unit on right click
        if (button == GLFW_MOUSE_BUTTON_RIGHT && down)
        {
            if (promptForDelete_()) {
                return true;
            }
        }
    }
    else if (m_titleTextBox && m_titleTextBox->visible())
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
                    triggerPortDrag_(inputLabel.first, false);
                    return false;
                }
                triggerPortDrop_(inputLabel.first, false);
                return false;
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
                    triggerPortDrag_(outputLabel.first, true);
                    return false;
                }
                triggerPortDrop_(outputLabel.first, true);
                return false;
            }
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (down)
        {
            // Open unit editor if mouse click is not captured above.
            triggerEditorCallback();
        }
        return false;
    }

    return false;
}

void synui::DefaultUnitWidget::onGridChange_()
{
    auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());

    int rowHeight = m_parentCircuit->getGridSpacing();

    // Special sizing for title label (so grid point will be centered relative to port labels)
    if (m_titleLabel) {
        int titleRowSize = rowHeight * 1.5;
        while (titleRowSize < 8)
            titleRowSize += rowHeight;
        int titleRowFontSize = titleRowSize > 12 ? titleRowSize - 2 : titleRowSize;
        l->setRowSize(0, titleRowSize);
        m_titleLabel->setFontSize(titleRowFontSize);
    }
    int portRowSize = m_parentCircuit->getGridSpacing();
    int portRowFontSize = portRowSize;


    const syn::Unit& unit = getUnit_();
    const auto& inputs = unit.inputs();
    const auto& outputs = unit.outputs();
    // Apply new row size to port rows
    for (int r = 0; r < l->rowCount() - 1; r++)
    {
        if (m_inputLabels.find(r) != m_inputLabels.end())
        {
            m_inputLabels[r]->setFontSize(portRowFontSize);
            if (portRowFontSize < 12)
            {
                m_inputLabels[r]->setCaption(inputs.name(r).substr(0, 2));
                m_inputLabels[r]->setFixedWidth(rowHeight);
                m_inputLabels[r]->setFixedHeight(rowHeight);
            }
            else
            {
                m_inputLabels[r]->setCaption(inputs.name(r));
                m_inputLabels[r]->setFixedWidth(0);
                m_inputLabels[r]->setFixedHeight(0);
            }
        }
        if (m_outputLabels.find(r) != m_outputLabels.end())
        {
            m_outputLabels[r]->setFontSize(portRowFontSize);
            if (portRowFontSize < 12)
            {
                m_outputLabels[r]->setCaption(outputs.name(r).substr(0, 2));
                m_outputLabels[r]->setFixedWidth(rowHeight);
                m_outputLabels[r]->setFixedHeight(rowHeight);
            }
            else
            {
                m_outputLabels[r]->setCaption(outputs.name(r));
                m_outputLabels[r]->setFixedWidth(0);
                m_outputLabels[r]->setFixedHeight(0);
            }
        }
        l->setRowSize(r + 1, rowHeight);
    }

    setSize({ 0,0 });
}

synui::SummingUnitWidget::SummingUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId) : UnitWidget(a_parent, a_vm, a_unitId), m_handleRadiusRatio(0.45f)
{
}

Eigen::Vector2i synui::SummingUnitWidget::getInputPortAbsPosition(int a_portId)
{
    int nInputPorts = m_vm->getUnit(m_unitId).numInputs() - 1;
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

int synui::SummingUnitWidget::getSelectedInputPort(const Eigen::Vector2i& p) const
{
    int nInputPorts = m_vm->getUnit(m_unitId).numInputs() - 1;
    float dAngle = DSP_PI*1.0f / (nInputPorts + 1);
    Vector2f center = size().cast<float>() * 0.5f;
    Vector2f relMouse = p.cast<float>() - center;
    float mouseAngle = atan2f(relMouse.y(), relMouse.x()) - DSP_PI*0.5;
    if (mouseAngle < 0)
        mouseAngle -= 2 * DSP_PI * floor(mouseAngle / (2.0f*DSP_PI));
    int slice = (mouseAngle - dAngle*0.5) / dAngle;
    return slice >= 0 && slice < nInputPorts ? slice : -1;
}

int synui::SummingUnitWidget::getSelectedOutputPort(const Eigen::Vector2i& p) const
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
    nanogui::Color bgColor(89, 63, 73, 255);
    nanogui::Color bgHighlightColor(25, 50);
    nanogui::Color handleColor(42, 43, 51, 255);
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

    // Draw plus sign
    nvgBeginPath(ctx);
    nvgMoveTo(ctx, size().x()*0.5, size().y()*0.5 - handleRadius);
    nvgLineTo(ctx, size().x()*0.5, size().y()*0.5 + handleRadius);
    nvgMoveTo(ctx, size().x()*0.5 - handleRadius, size().y() * 0.5);
    nvgLineTo(ctx, size().x()*0.5 + handleRadius, size().y() * 0.5);
    nvgStrokeColor(ctx, plusColor);
    nvgStrokeWidth(ctx, 2.0f);
    nvgStroke(ctx);

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
        const int* portIndices = unit.inputs().indices();
        int selectedInputPort = getSelectedInputPort(mousePos);

        // Use the port selected by the mouse if it is free, otherwise find a free one.
        if (selectedInputPort >= 0 && unit.inputSource(portIndices[selectedInputPort]) == nullptr) {
            selectedPort = selectedInputPort;
        }
        else {
            for (int i = 0; i < unit.numInputs(); i++)
            {
                int inputId = unit.inputs().indices()[i];
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
        if (down && !isHandleSelected(mousePos))
        {
            triggerPortDrag_(selectedPort, isOutputSelected);
            return false;
        }
        if (!down) {
            triggerPortDrop_(selectedPort, isOutputSelected);
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
    nanogui::Color bgColor(89, 63, 73, 255);
    nanogui::Color bgHighlightColor(25, 50);
    nanogui::Color handleColor(42, 43, 51, 255);
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

    // Draw X sign
    nvgBeginPath(ctx);
    nvgTranslate(ctx, size().x()*0.5, size().y()*0.5);
    nvgRotate(ctx, DSP_PI*0.25);
    nvgMoveTo(ctx, 0.0f, -handleRadius);
    nvgLineTo(ctx, 0.0f, handleRadius);
    nvgMoveTo(ctx, -handleRadius, 0.0f);
    nvgLineTo(ctx, handleRadius, 0.0f);
    nvgStrokeColor(ctx, plusColor);
    nvgStrokeWidth(ctx, 2.0f);
    nvgStroke(ctx);

    nvgRestore(ctx);
}
