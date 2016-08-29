#include "UIDefaultUnitControl.h"
#include "UITextSlider.h"
#include "UIDigitControl.h"
#include "UILayout.h"
#include "UILabel.h"

synui::UIDefaultUnitControl::UIDefaultUnitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId) :
	UIUnitControl(a_window, a_vm, a_unitId)
{
	setLayout(new GroupLayout(2,2,4,5));

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
		UILabel* m_label = add<UILabel>("labels");
		m_label->setText(paramCtrl->getName());
		m_label->setVisible(paramCtrl->visible());
		addChild(paramCtrl,"controls");
	}

}

void synui::UIDefaultUnitControl::draw(NVGcontext* a_nvg)
{
	const syn::Unit& unit = m_vm->getUnit(m_unitId);
	int i = 0;
	bool isDirty = false;
	for (UIParamControl* paramControl : m_paramControls)
	{
		const syn::UnitParameter& param = unit.getParameter(i);
		UILabel* lbl = static_cast<UILabel*>(getGroup("labels")[i]);
		if (param.isVisible() != paramControl->visible())
		{
			paramControl->setVisible(param.isVisible());
			lbl->setVisible(paramControl->visible());
			isDirty = true;
		}
		i++;
	}
	if (isDirty) {
		performLayout(a_nvg);
	}
}

void synui::UIDefaultUnitControl::_onResize()
{
	performLayout(m_window->getContext());
}