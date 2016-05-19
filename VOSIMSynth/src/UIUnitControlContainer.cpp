#include "UIUnitControlContainer.h"
#include "UICircuitPanel.h"
#include "UIUnitPort.h"
#include "UICell.h"
#include "UIButton.h"
#include "UIUnitControl.h"
#include "Theme.h"

syn::UIUnitControlContainer::UIUnitControlContainer(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, UIUnitControl* a_unitControl):
	UIWindow{a_window, a_vm->getUnit(a_unitId).getName()},
	m_vm(a_vm),
	m_unitId(a_unitId),
	m_unitControl(a_unitControl) {
	m_row = new UIRow(m_window);
	m_cols[0] = new UICol(m_window);
	m_cols[1] = new UICol(m_window);

	const Unit& unit = m_vm->getUnit(m_unitId);
	int nIn = unit.getNumInputs();
	int nOut = unit.getNumOutputs();

	for (int i = 0; i < nIn; i++) {
		m_inPorts.push_back(new UIUnitPort(a_window, a_vm, a_unitId, i, true));
		m_cols[0]->addChild(m_inPorts[i]);
	}

	for (int i = 0; i < nOut; i++) {
		m_outPorts.push_back(new UIUnitPort(a_window, a_vm, a_unitId, i, false));
		m_cols[1]->addChild(m_outPorts[i]);
	}

	
	m_row->addChild(m_cols[0]);
	m_row->addChild(a_unitControl);
	m_row->addChild(m_cols[1]);
	m_row->setChildResizePolicy(UICell::CNONE);
	m_row->setSelfResizePolicy(UICell::SACTIVE);
	m_row->setPadding({ 0,1,0,1 });
	m_row->setChildrenSpacing(5);
	addChildToBody(m_row);

	m_closeButton = new UIButton(a_window, "", 0xE729);
	m_closeButton->setCallback([&]() {
		this->close();
	});
	m_closeButton->setSize({theme()->mWindowHeaderHeight - 4 ,theme()->mWindowHeaderHeight - 4});
	m_closeButton->setFontSize(8);
	addChildToHeader(m_closeButton);
}

syn::UIUnitPort* syn::UIUnitControlContainer::getSelectedInPort(const Vector2i& a_absPt) const {
	for (UIUnitPort* port : m_inPorts) {
		Vector2i relPos = a_absPt - port->parent()->getAbsPos();
		if (port->contains(relPos))
			return port;
	}
	return nullptr;
}

syn::UIUnitPort* syn::UIUnitControlContainer::getSelectedOutPort(const Vector2i& a_absPt) const {
	for (UIUnitPort* port : m_outPorts) {
		Vector2i relPos = a_absPt - port->parent()->getAbsPos();
		if (port->contains(relPos))
			return port;
	}
	return nullptr;
}

syn::UIComponent* syn::UIUnitControlContainer::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* child = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (child)
		return child;
	return this;
}

void syn::UIUnitControlContainer::close() const {
	m_window->getCircuitPanel()->requestDeleteUnit(m_unitId);
}
