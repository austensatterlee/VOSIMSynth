#include "UIDefaultUnitControl.h"

syn::UIDefaultUnitControl::UIDefaultUnitControl(MainWindow* a_window, VoiceManager* a_vm, int a_unitId) :
	UIUnitControl(a_window, a_vm, a_unitId)
{
	m_col = new UICol(a_window);
	m_col->setChildResizePolicy(UICell::CMAX);
	m_col->setSelfResizePolicy(UICell::SRNONE);
	addChild(m_col);

	const Unit& unit = a_vm->getUnit(a_unitId);
	int nParams = unit.getNumParameters();
	for (int i = 0; i < nParams; i++) {
		UITextSlider* paramControl = new UITextSlider(a_window, m_vm, m_unitId, i);
		m_paramControls.push_back(paramControl);
		paramControl->setVisible(unit.getParameter(i).isVisible());
		m_col->addChild(paramControl);
	}
	m_col->pack();
}

void syn::UIDefaultUnitControl::notifyChildResized(UIComponent* a_child) {
	setMinSize(m_col->minSize());
}

void syn::UIDefaultUnitControl::draw(NVGcontext* a_nvg)
{
	const Unit& unit = m_vm->getUnit(m_unitId);
	int i = 0;
	bool isDirty = false;
	for (UITextSlider* txt_slider : m_paramControls)
	{
		const UnitParameter& param = unit.getParameter(i);
		if (param.isVisible() != txt_slider->visible())
		{
			txt_slider->setVisible(param.isVisible());
			isDirty = true;
		}
		i++;
	}
	if (isDirty) {
		m_col->pack();
	}
}

void syn::UIDefaultUnitControl::_onResize()
{
	m_col->setSize(size());
}