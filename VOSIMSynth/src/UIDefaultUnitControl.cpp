#include "UIDefaultUnitControl.h"

Eigen::Vector2i syn::DefaultUnitControl::calcAutoSize(NVGcontext* a_nvg) const {
	Vector2i size = { 0,0 };
	for (UITextSlider* child : m_paramControls) {
		if (child->visible()) {
			size[0] = MAX(size[0], child->calcAutoSize(a_nvg)[0]);
			size[1] += child->calcAutoSize(a_nvg)[1];
		}
	}
	
	return size;
}

void syn::DefaultUnitControl::onResize() {
	m_isSizeDirty = true;
}

void syn::DefaultUnitControl::draw(NVGcontext* a_nvg) {
	if(m_isSizeDirty) {
		m_size[0] = MAX(size()[0], calcAutoSize(a_nvg)[0]);

		for (UITextSlider* child : m_paramControls) {
			child->setSize({ size()[0],child->size()[1] });
		}
		m_isSizeDirty = false;
	}
}
