#include "UIScrollPanel.h"

synui::UIScrollPanel::UIScrollPanel(MainWindow* a_window): UIComponent(a_window), m_minExtent{0,0}, m_maxExtent{0,0}, m_scrollOffset{0,0} {}

void synui::UIScrollPanel::scroll(const Vector2i& a_amt) {
	setScrollPos(m_scrollOffset + a_amt);
}

void synui::UIScrollPanel::setScrollPos(const Vector2i& a_pos) {
	m_scrollOffset = a_pos;
	m_scrollOffset = m_scrollOffset.cwiseMax(m_minExtent);
	m_scrollOffset = m_scrollOffset.cwiseMin(m_maxExtent);
	for (shared_ptr<UIComponent> child : m_children) {
		updateChildPosition_(child);
	}
}

Eigen::Vector2i synui::UIScrollPanel::getScrollOffset() const {
	return m_scrollOffset;
}

bool synui::UIScrollPanel::onMouseScroll(const UICoord& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) {
	if (UIComponent::onMouseScroll(a_relCursor, a_diffCursor, a_scrollAmt))
		return true;
	scroll({0,10 * a_scrollAmt});
	return true;
}

void synui::UIScrollPanel::setChildrenStyles(NVGcontext* a_nvg) {
	nvgIntersectScissor(a_nvg, 0, 0, size().x(), size().y());
}

void synui::UIScrollPanel::updateExtents_() {
	for (shared_ptr<UIComponent> child : m_children) {
		m_minExtent = m_minExtent.cwiseMin(m_originalPositions[child.get()]);
		m_maxExtent = m_maxExtent.cwiseMax(m_originalPositions[child.get()] + child->size() - size());
	}
	m_scrollOffset = m_scrollOffset.cwiseMax(m_minExtent);
	m_scrollOffset = m_scrollOffset.cwiseMin(m_maxExtent);
}

void synui::UIScrollPanel::updateChildPosition_(shared_ptr<UIComponent> a_child) {
	m_dirtyMap[a_child.get()] = false;
	a_child->setRelPos(m_originalPositions[a_child.get()] - m_scrollOffset);
	m_dirtyMap[a_child.get()] = true;
}

void synui::UIScrollPanel::notifyChildResized(UIComponent* a_child) {
	updateExtents_();
}

void synui::UIScrollPanel::_onAddChild(shared_ptr<UIComponent> a_newchild) {
	m_dirtyMap[a_newchild.get()] = true;
	_onChildMoved(a_newchild.get());
}

void synui::UIScrollPanel::_onRemoveChild() {
	updateExtents_();
}

void synui::UIScrollPanel::_onChildMoved(UIComponent* a_child) {
	if (m_dirtyMap[a_child]) {
		m_originalPositions[a_child] = a_child->getRelPos();
		updateChildPosition_(a_child->getSharedPtr());
	}
	updateExtents_();
}
