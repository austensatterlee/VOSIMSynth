#include "UIComponent.h"

namespace syn
{


	UIComponent::UIComponent(const UIComponent& a_other) {
		if (a_other.m_parent) {
			a_other.m_parent->addChild(this);
		}
	}

	void UIComponent::grow(const NDPoint<2, int>& a_amt) {
		m_size += a_amt;
	}
}