#include "UnitEditor.h"
#include "VoiceManager.h"
#include "UI.h"

void synui::UnitEditor::_build()
{
    int controlFontSize = 13;
    int titleFontSize = 20;

    using Anchor = nanogui::AdvancedGridLayout::Anchor;

    for (int i = 0; i < childCount(); i++) { removeChild(i); }

    auto newLayout = new nanogui::AdvancedGridLayout({ 10, 0, 10, 0 }, {});
    newLayout->setMargin(5);
    newLayout->setColStretch(2, 1);
    setLayout(newLayout);
    setVisible(true);

    nanogui::FormHelper* helper = new nanogui::FormHelper(nullptr);
    helper->setWidget(this);

    /* Add title */
    auto titleLbl = new nanogui::Label(this, m_vm->getUnit(m_unitId).name(), "sans-bold", titleFontSize);
    titleLbl->setTextAlign(nanogui::Label::Alignment::Center);
    helper->addWidget("", titleLbl);


    /* Add parameter controls */
    const syn::Unit& unit = m_vm->getUnit(m_unitId);
    const int* paramIndices = unit.parameters().indices();
    int nParams = unit.numParams();
    for (int pNum = 0; pNum < nParams; pNum++)
    {
        int paramId = paramIndices[pNum];
        const syn::UnitParameter& param = unit.param(paramId);
        const string& paramName = param.getName();
        syn::UnitParameter::EParamType paramType = param.getType();
        syn::UnitParameter::EControlType paramControlType = param.getControlType();
        const string& paramUnits = param.getUnitsString();
        switch (paramType)
        {
        case syn::UnitParameter::Null:
            continue;
            break;
        case syn::UnitParameter::Bool:
        {
            auto setter = [this, paramId](bool val) { setParamValue(paramId, val); };

            nanogui::CheckBox* cb = new nanogui::CheckBox(this, "", setter);

            cb->setFontSize(controlFontSize);

            auto getter = [this, cb, paramId]()
            {
                auto param = m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).param(paramId);
                bool value = param.getBool();
                bool visible = param.isVisible();
                if (cb->checked() != value || cb->visible() != visible)
                {
                    m_isDirty = true;
                    cb->setChecked(value);
                    cb->setVisible(visible);
                }
            };

            m_controls[paramId] = cb;
            m_refreshFuncs[paramId] = getter;
            break;
        }
        case syn::UnitParameter::Enum:
        {
            // Build item list
            const std::vector<syn::UnitParameter::DisplayText>& options = param.getDisplayTexts();
            std::vector<string> items{ options.size() };
            std::transform(options.begin(), options.end(), items.begin(), [](const syn::UnitParameter::DisplayText& dt) { return dt.m_text; });

            nanogui::ComboBox* cb = new nanogui::ComboBox(this, items);
            cb->setFontSize(controlFontSize);

            auto setter = [this, cb, paramId](int val) { setParamFromString(paramId, cb->items()[val]); };
            auto getter = [this, cb, paramId]()
            {
                auto param = m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).param(paramId);
                int value = param.getInt();
                bool visible = param.isVisible();
                if (cb->selectedIndex() != value || cb->visible() != visible)
                {
                    m_isDirty = true;
                    cb->setSelectedIndex(value);
                    cb->setVisible(visible);
                }
            };

            cb->setCallback(setter);
            m_controls[paramId] = cb;
            m_refreshFuncs[paramId] = getter;
            break;
        }
        case syn::UnitParameter::Int:
        {
            auto setter = [this, paramId](const int& val) { setParamValue(paramId, val); };

            nanogui::IntBox<int>* ib = new nanogui::IntBox<int>(this, 0);
            ib->setMinValue(param.getMin());
            ib->setMaxValue(param.getMax());
            ib->setUnits(param.getUnitsString());
            ib->setSpinnable(true);
            ib->setEditable(true);
            ib->setCallback(setter);
            ib->setAlignment(nanogui::TextBox::Alignment::Right);

            auto getter = [this, ib, paramId]()
            {
                auto param = m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).param(paramId);
                int value = param.getInt();
                bool visible = param.isVisible();
                if (ib->value() != value || ib->visible() != visible)
                {
                    m_isDirty = true;
                    ib->setValue(value);
                    ib->setVisible(visible);
                }
            };

            m_controls[paramId] = ib;
            m_refreshFuncs[paramId] = getter;
            break;
        }
        case syn::UnitParameter::Double:
        {
            Widget* control = new Widget(this);
            control->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 5));

            /* Build float box */
            nanogui::FloatBox<double>* fb = new nanogui::FloatBox<double>(control, 0.0);
            fb->setMinValue(param.getMin());
            fb->setMaxValue(param.getMax());
            fb->setUnits(param.getUnitsString());
            fb->setValueIncrement(std::pow(10.0, -param.getPrecision()));
            fb->setSpinnable(true);
            fb->setEditable(true);
            fb->setAlignment(nanogui::TextBox::Alignment::Right);

            auto fb_setter = [this, paramId](const string& val)
            {
                setParamFromString(paramId, val);
                return true;
            };
            static_cast<nanogui::TextBox*>(fb)->setCallback(fb_setter);

            /* Build slider for "Bounded" parameters */
            nanogui::Slider* s = paramControlType == syn::UnitParameter::Bounded ? new nanogui::Slider(control) : nullptr;
            
            if (s) {
                s->setRange({ 0.0,1.0 });

                auto s_setter = [this, paramId](float f)
                {
                    setParamNorm(paramId, f);
                };
                s->setCallback(s_setter);
            }

            /* Create getter function */
            auto getter = [this, control, s, fb, paramId]()
            {
                auto param = m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).param(paramId);
                string text = param.getValueString();
                double norm = param.getNorm();
                bool visible = param.isVisible();

                int sig = param.getPrecision();
                double valueIncr = std::pow(10, -sig);

                bool isDirty = static_cast<nanogui::TextBox*>(fb)->value() != text || control->visible() != visible || fb->getValueIncrement() != valueIncr || (s && norm != s->value());               
                if (isDirty)
                {
                    m_isDirty = true;
                    static_cast<nanogui::TextBox*>(fb)->setValue(text);
                    control->setVisible(visible);

                    if (sig <= 0)
                        fb->numberFormat("%.0f");
                    else
                        fb->numberFormat("%." + std::to_string(sig) + "f");

                    fb->setValueIncrement(valueIncr);

                    if (s) {
                        s->setValue(norm);
                        s->setVisible(visible);
                        s->setTooltip(param.getValueString() + " " + param.getUnitsString());
                    }
                }
            };

            m_controls[paramId] = control;
            m_refreshFuncs[paramId] = getter;
            break;
        }
        default:
            continue;
            break;
        }

        auto lbl = new nanogui::Label(this, paramName);
        m_controlLabels[paramId] = lbl;
        newLayout->appendRow(0);
        newLayout->setAnchor(lbl, Anchor{ 0,newLayout->rowCount() - 1,4,1 });
        newLayout->appendRow(0);
        newLayout->setAnchor(m_controls[paramId], Anchor{ 1,newLayout->rowCount() - 1,3,1 });
    }
}

void synui::UnitEditor::setParamValue(int a_paramId, double a_val) const
{
    syn::RTMessage* msg = new syn::RTMessage();
    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
    {
        UnitEditor* self;
        double val;
        int unitId, paramId;
        GetArgs(a_data, 0, self, unitId, paramId, val);
        a_circuit->getUnit(unitId).param(paramId).set(val);
        if (a_isLast)
            self->m_isDirty = true;
    };
    PutArgs(&msg->data, this, m_unitId, a_paramId, a_val);
    m_vm->queueAction(msg);
}

void synui::UnitEditor::setParamNorm(int a_paramId, double a_normval) const
{
    syn::RTMessage* msg = new syn::RTMessage();
    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
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

void synui::UnitEditor::nudgeParam(int a_paramId, double a_logScale, double a_linScale) const
{
    syn::RTMessage* msg = new syn::RTMessage();
    PutArgs(&msg->data, m_unitId, a_paramId, a_logScale, a_linScale);
    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
    {
        double logScale, linScale;
        int unitId, paramId;
        GetArgs(a_data, 0, unitId, paramId, logScale, linScale);
        a_circuit->getUnit(unitId).param(paramId).nudge(logScale, linScale);
    };
    m_vm->queueAction(msg);
}

void synui::UnitEditor::setParamFromString(int a_paramId, const string& a_str) const
{
    syn::RTMessage* msg = new syn::RTMessage();
    PutArgs(&msg->data, this, a_str, m_unitId, a_paramId);
    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
    {
        UnitEditor* self;
        string valStr;
        int unitId, paramId;
        GetArgs(a_data, 0, self, valStr, unitId, paramId);
        a_circuit->getUnit(unitId).param(paramId).setFromString(valStr);
        if (a_isLast)
            self->m_isDirty = true;
    };
    m_vm->queueAction(msg);
}

void synui::UnitEditor::draw(NVGcontext* ctx)
{
    for (auto x : m_refreshFuncs)
    {
        x.second();
        m_controlLabels[x.first]->setVisible(m_controls[x.first]->visible());
    }
    if (m_isDirty) {
        parent()->performLayout(ctx);
        m_isDirty = false;
    }

    Widget::draw(ctx);
}


synui::UnitEditorHost::UnitEditorHost(Widget* parent, syn::VoiceManager* a_vm) : StackedWidget(parent), m_vm(a_vm), m_activeUnitId(-1) {}

void synui::UnitEditorHost::addEditor(unsigned a_classId, int a_unitId)
{
    // Remove any editor already associated with this unit.
    if (m_editorMap.find(a_unitId) != m_editorMap.end())
        removeEditor(a_unitId);
    
    // Construct the new editor.
    UnitEditorConstructor constructor = [](Widget* p, syn::VoiceManager* vm, int unitId) { return new UnitEditor(p, vm, unitId); };
    if (m_registeredUnitEditors.find(a_classId) != m_registeredUnitEditors.end())
        constructor = m_registeredUnitEditors[a_classId];
    UnitEditor* editor = constructor(this, m_vm, a_unitId);
    editor->setVisible(false);

    Widget::addChild(childCount(), editor);
    m_editorMap[a_unitId] = editor;
}

void synui::UnitEditorHost::removeEditor(int a_unitId)
{
    if (m_editorMap.find(a_unitId) == m_editorMap.end())
        return;

    UnitEditor* editor = m_editorMap[a_unitId];
    m_editorMap.erase(a_unitId);
    if (selectedIndex() == childIndex(editor)){
        setSelectedIndex(-1);
        m_activeUnitId = -1;
        removeChild(editor);
    }
    else
    {    
        Widget* selectedWidget = childAt(selectedIndex());    
        removeChild(editor);
        int newIndex = childIndex(selectedWidget);
        setSelectedIndex(newIndex);
    }
}

void synui::UnitEditorHost::activateEditor(unsigned a_classId, int a_unitId)
{
    // Create this unit's editor if it doesn't already exist.
    if (m_editorMap.find(a_unitId) == m_editorMap.end())
        addEditor(a_classId, a_unitId);

    // Make this editor the visible one.
    UnitEditor* editor = m_editorMap[a_unitId];
    setSelectedIndex(childIndex(editor));
    m_activeUnitId = a_unitId;

    screen()->performLayout();

    // If the editor host is in a tab widget, set the active tab to show the editor.
    Widget* w = this;
    while (w->parent())
    {
        auto tabContent = dynamic_cast<nanogui::StackedWidget*>(w->parent());
        if (tabContent && tabContent->parent())
        {
            auto tabWidget = dynamic_cast<nanogui::TabWidget*>(tabContent->parent());
            if (tabWidget)
            {
                int tabIndex = tabContent->childIndex(w);
                tabWidget->setActiveTab(tabIndex);
                break;
            }
        }
        w = w->parent();
    }
}
