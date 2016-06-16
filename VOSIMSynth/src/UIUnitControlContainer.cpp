#include "UIUnitControlContainer.h"
#include "UICircuitPanel.h"
#include "UIUnitPort.h"
#include "UICell.h"
#include "UIButton.h"
#include "UIUnitControl.h"
#include "Theme.h"
#include <entypo.h>

syn::UIUnitControlContainer::UIUnitControlContainer(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, UIUnitControl* a_unitControl) :
	UIWindow{ a_window, a_vm->getUnit(a_unitId).getName() },
	m_vm(a_vm),
	m_unitId(a_unitId),
	m_unitControl(a_unitControl)
{
	m_bodyRow->setChildResizePolicy(UICell::CNONE);
	m_bodyRow->setChildrenSpacing(2);
	m_cols[0] = new UICol(m_window);
	m_cols[0]->setChildResizePolicy(UICell::CMATCHMIN);
	m_cols[1] = new UICol(m_window);
	m_cols[1]->setChildResizePolicy(UICell::CMATCHMIN);

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

	addChildToBody(m_cols[0]);
	addChildToBody(a_unitControl);
	addChildToBody(m_cols[1]);
	//m_bodyRow->setGreedyChild(m_cols[1], NVG_ALIGN_RIGHT);

	m_closeButton = new UIButton(a_window, "", ENTYPO_TRASH);
	m_closeButton->setCallback([&]() {
		this->close();
	});
	m_closeButton->setRelPos({ m_closeButton->getRelPos().x(), 2 });
	m_closeButton->setSize({ theme()->mWindowHeaderHeight, theme()->mWindowHeaderHeight-6 });
	m_closeButton->setFontSize(10);
	addChildToHeader(m_closeButton);
}

syn::UIUnitPort* syn::UIUnitControlContainer::getSelectedInPort(const UICoord& a_pt) const {
	for (UIUnitPort* port : m_inPorts) {
		if (port->contains(a_pt))
			return port;
	}
	return nullptr;
}

syn::UIUnitPort* syn::UIUnitControlContainer::getSelectedOutPort(const UICoord& a_pt) const {
	for (UIUnitPort* port : m_outPorts) {
		if (port->contains(a_pt))
			return port;
	}
	return nullptr;
}

syn::UIComponent* syn::UIUnitControlContainer::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* child = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (child)
		return child;
	return this;
}

void syn::UIUnitControlContainer::close() const {
	m_window->getCircuitPanel()->requestDeleteUnit(m_unitId);
}