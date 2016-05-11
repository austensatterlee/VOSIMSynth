#include "UIDefaultUnitControl.h"

syn::DefaultUnitControl::DefaultUnitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId): UIUnitControl{a_window, a_vm, a_unitId} {
	m_col = new UICol(a_window);	
	const Unit& unit = a_vm->getUnit(a_unitId);
	int nParams = unit.getNumParameters();
	for (int i = 0; i < nParams; i++) {
		UITextSlider* paramControl = new UITextSlider(m_window, m_vm, m_unitId, i);
		m_paramControls.push_back(paramControl);
		m_col->addChild(paramControl);
	}

	addChild(m_col);
	m_col->setChildResizePolicy(UICol::CMATCHMAX);
	m_col->setSelfResizePolicy(UICol::SFIT);
	m_col->pack();

	setMinSize_(m_col->minSize());
}

void syn::DefaultUnitControl::notifyChildResized(UIComponent* a_child) {
	setMinSize_(m_col->minSize());	
}

void syn::DefaultUnitControl::_onResize() {
	m_col->setSize(size());	
}
