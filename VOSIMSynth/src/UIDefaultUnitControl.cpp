#include "UIDefaultUnitControl.h"

Eigen::Vector2i syn::DefaultUnitControl::calcAutoSize() const {
	Vector2i size = { 0,0 };
	for (UITextSlider* child : m_paramControls) {
		if (child->visible()) {
			size[0] = MAX(size[0], child->calcAutoSize()[0]);
			size[1] += child->calcAutoSize()[1];
		}
	}
	
	return size;
}

void syn::DefaultUnitControl::onResize() {
	m_size[0] = MAX(m_size[0], calcAutoSize()[0]);
	
	for (UITextSlider* child : m_paramControls) {		
		child->setSize({ m_size[0],child->size()[1] });
	}
}

void syn::DefaultUnitControl::draw(NVGcontext* a_nvg) {

}
