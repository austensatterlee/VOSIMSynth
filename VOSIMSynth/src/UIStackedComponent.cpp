#include "UIStackedComponent.h"

namespace synui
{
	UIStackedComponent::UIStackedComponent(MainWindow *a_window)
		: UIComponent(a_window) { }

	void UIStackedComponent::setSelectedIndex(int index) {
		assert(index < numChildren());
		if (m_selectedIndex >= 0)
			m_childMap[m_selectedIndex]->setVisible(false);
		m_selectedIndex = index;
		m_childMap[m_selectedIndex]->setVisible(true);
	}

	int UIStackedComponent::selectedIndex() const {
		return m_selectedIndex;
	}

	void UIStackedComponent::performLayout(NVGcontext *ctx) {
		for (auto child : m_children) {
			child->setRelPos(Vector2i::Zero());
			child->setSize(size());
			child->performLayout(ctx);
		}
	}

	Vector2i UIStackedComponent::calcMinSize(NVGcontext *ctx) const {
		Vector2i size = Vector2i::Zero();
		for (auto child : m_children)
			size = size.cwiseMax(child->minSize());
		return size;
	}

	void UIStackedComponent::addChild(int index, UIComponent *child) {
		if (m_selectedIndex >= 0)
			m_children[m_selectedIndex]->setVisible(false);
		UIComponent::addChild(child);
		m_childMap[index] = child;
		m_revChildMap[child] = index;
		child->setVisible(true);
		setSelectedIndex(index);
	}

	int UIStackedComponent::childIndex(UIComponent* a_comp) const {
		return m_revChildMap.at(a_comp);
	}

	void UIStackedComponent::notifyChildResized(UIComponent* a_child) {
		setMinSize(calcMinSize(m_window->getContext()));
		performLayout(m_window->getContext());
	}

	void UIStackedComponent::_onResize() {
		setMinSize(calcMinSize(m_window->getContext()));
		performLayout(m_window->getContext());
	}
}
