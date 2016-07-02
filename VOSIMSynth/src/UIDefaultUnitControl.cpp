#include "UIDefaultUnitControl.h"
#include "UITextSlider.h"
#include "UIDigitControl.h"

synui::UIDefaultUnitControl::UIDefaultUnitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId) :
	UIUnitControl(a_window, a_vm, a_unitId)
{
	m_col = new UICol(a_window);
	m_col->setChildResizePolicy(UICell::CMAX);
	m_col->setSelfResizePolicy(UICell::SRNONE);
	addChild(m_col);

	const syn::Unit& unit = a_vm->getUnit(a_unitId);
	int nParams = unit.getNumParameters();
	for (int i = 0; i < nParams; i++) {
		UIParamControl* paramCtrl;
		if (unit.getParameter(i).getType() == syn::UnitParameter::Double || unit.getParameter(i).getType() == syn::UnitParameter::Int) {
			switch (unit.getParameter(i).getControlType()) {
			case syn::UnitParameter::Unbounded:
				paramCtrl = new UIDigitControl(a_window, m_vm, m_unitId, i);
				break;
			case syn::UnitParameter::Bounded:
			default:
				paramCtrl = new UITextSlider(a_window, m_vm, m_unitId, i);
				break;
			}
		}else {
			paramCtrl = new UITextSlider(a_window, m_vm, m_unitId, i);			
		}
		m_paramControls.push_back(paramCtrl);
		paramCtrl->setVisible(unit.getParameter(i).isVisible());
		m_col->addChild(paramCtrl);
	}
	m_col->pack();
}

void synui::UIDefaultUnitControl::notifyChildResized(UIComponent* a_child) {
	setMinSize(m_col->minSize());
}

void synui::UIDefaultUnitControl::draw(NVGcontext* a_nvg)
{
	const syn::Unit& unit = m_vm->getUnit(m_unitId);
	int i = 0;
	bool isDirty = false;
	for (UIParamControl* paramControl : m_paramControls)
	{
		const syn::UnitParameter& param = unit.getParameter(i);
		if (param.isVisible() != paramControl->visible())
		{
			paramControl->setVisible(param.isVisible());
			isDirty = true;
		}
		i++;
	}
	if (isDirty) {
		m_col->pack();
	}
}

void synui::UIDefaultUnitControl::_onResize()
{
	m_col->setSize(size());
}