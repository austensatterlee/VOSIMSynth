#include "vosimsynth/widgets/UnitWidget.h"
#include "vosimsynth/widgets/CircuitWidget.h"
#include "vosimsynth/SystemWindow.h"
#include "vosimsynth/VOSIMSynthGUI.h"
#include <vosimlib/Unit.h>
#include <vosimlib/VoiceManager.h>
#include <vosimlib/Command.h>

synui::UnitWidget::UnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
    : Widget(a_parent),
      m_parentCircuit(a_parent),
      m_vm(a_vm),
      m_unitId(a_unitId),
      m_classIdentifier(getUnit_().getClassIdentifier()),
      m_name(""),
      m_highlighted(false) {
    setDraggable(false);
    setTooltip(getUnit_().getClassName());
}

synui::UnitWidget::~UnitWidget() {}

const string& synui::UnitWidget::getName() const {
    return m_name;
}

void synui::UnitWidget::setName(const string& a_name) {
    m_name = a_name;
    auto f = [this]() {
        for (int i = 0; i < m_vm->getMaxVoices(); i++) {
            m_vm->getVoiceCircuit(i).getUnit(m_unitId).setName(m_name);
        }
        m_vm->getPrototypeCircuit().getUnit(m_unitId).setName(m_name);
    };
    m_vm->queueAction(syn::MakeCommand(f));
}

synui::UnitWidget::operator json() const {
    json j;
    j["x"] = position().x();
    j["y"] = position().y();
    j["name"] = getName();
    return j;
}

synui::UnitWidget* synui::UnitWidget::load(const json& j) {
    if (j.find("name") != j.end())
        setName(j["name"].get<string>());
    return this;
}

const syn::Unit& synui::UnitWidget::getUnit_() const { return m_vm->getUnit(m_unitId); }

bool synui::UnitWidget::promptForDelete_() {
    if (getUnitId() != m_vm->getPrototypeCircuit().getInputUnitId() && getUnitId() != m_vm->getPrototypeCircuit().getOutputUnitId()) {
        auto dlg = new nanogui::MessageDialog(screen(), nanogui::MessageDialog::Type::Question, "Confirm deleteion", "Delete " + getName() + "?", "Delete", "Cancel", true);
        dlg->setCallback([this](int x) {
            if (x == 0) {
                m_parentCircuit->deleteUnit(m_unitId);
            }
        });
        return true;
    }
    return false;
}
