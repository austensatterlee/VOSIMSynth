#include "UIDefaultUnitControl.h"

syn::DefaultUnitControl::DefaultUnitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId): UIUnitControl{a_window, a_vm, a_unitId} {
	const Unit& unit = a_vm->getUnit(a_unitId);
	int nParams = unit.getNumParameters();
	for (int i = 0; i < nParams; i++) {
		UITextSlider* paramControl = new UITextSlider(m_window, m_vm, m_unitId, i);
		addChild(paramControl);
		m_paramControls.push_back(paramControl);
		paramControl->setSize({0,theme()->mTextSliderFontSize });
		paramControl->setRelPos({0,i*theme()->mTextSliderFontSize});
	}

	Vector2i minSize = { 0,0 };
	for (UITextSlider* child : m_paramControls) {
		if (child->visible()) {
			minSize[0] = MAX(minSize[0], child->size()[0]);
			minSize[1] += child->size()[1];
		}
	}
	setMinSize_(minSize);
}

void syn::DefaultUnitControl::_onResize() {
	m_isSizeDirty = true;
}

void syn::DefaultUnitControl::draw(NVGcontext* a_nvg) {
	if(m_isSizeDirty) {
		for (UITextSlider* child : m_paramControls) {
			child->setSize({ size()[0],child->size()[1] });
		}
		m_isSizeDirty = false;
	}
}
