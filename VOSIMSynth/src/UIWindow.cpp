#include "UIWindow.h"
#include "UICell.h"
#include "UILabel.h"
#include "Theme.h"

syn::UIWindow::UIWindow(VOSIMWindow* a_window, const string& a_title) :
	UIComponent{ a_window },
	m_isCollapsed(false) {
	m_col = new UICol(a_window);
	addChild(m_col);

	m_headerRow = new UIRow(a_window);
	m_bodyRow = new UIRow(a_window);
	m_bodyRow->setRelPos({ 0,theme()->mWindowHeaderHeight });
	m_bodyRow->setPadding({ 0,5,0,5 });
	m_col->setChildResizePolicy(UICell::CMATCHMIN);
	m_col->setPadding({ 5,0,5,0 });
	m_col->addChild(m_headerRow);
	m_col->addChild(m_bodyRow);

	m_title = new UILabel(a_window);
	m_title->setText(a_title);
	m_title->setFontSize(16.0);
	m_title->setFontColor(theme()->mTextColor);
	addChildToHeader(m_title);

	m_headerRow->setGreedyChild(m_title, NVG_ALIGN_LEFT);
	m_headerRow->setSize({ -1,theme()->mWindowHeaderHeight });
}

void syn::UIWindow::addChildToHeader(UIComponent* a_newChild) const {
	m_headerRow->addChild(a_newChild);
}

void syn::UIWindow::addChildToBody(UIComponent* a_newChild) const {
	m_bodyRow->addChild(a_newChild);
}

bool syn::UIWindow::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	if (UIComponent::onMouseDrag(a_relCursor, a_diffCursor))
		return true;
	move(a_diffCursor);
	return true;
}

syn::UIComponent* syn::UIWindow::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* child = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (child)
		return child;

	if (a_relCursor.y() - m_pos.y() < theme()->mWindowHeaderHeight) {
		if (a_isDblClick) {
			toggleCollapsed();
		}
		else {
			return this;
		}
	}
	return nullptr;
}

void syn::UIWindow::notifyChildResized(UIComponent* a_child) {
	Vector2i minSize = { 0,0 };
	Vector2i newSize = { 0,0 };
	for (shared_ptr<UIComponent> child : m_children) {
		Vector2i childExtent = child->getRelPos() + child->size();
		Vector2i minChildExtent = child->getRelPos() + child->minSize();
		newSize = newSize.cwiseMax(childExtent);
		minSize = minSize.cwiseMax(minChildExtent);
	}
	setMinSize(minSize);
	setSize(newSize);
}

void syn::UIWindow::toggleCollapsed() {
	if (m_isCollapsed)
		expand();
	else
		collapse();
}

void syn::UIWindow::collapse() {
	m_isCollapsed = true;
	m_oldSize = m_size;
	m_oldMinSize = minSize();
	m_bodyRow->setVisible(false);
	setMinSize({ m_size[0], theme()->mWindowHeaderHeight });
	setSize({ m_size[0],theme()->mWindowHeaderHeight });
}

void syn::UIWindow::expand() {
	m_isCollapsed = false;
	setMinSize(m_oldMinSize);
	setSize(m_oldSize);
	m_bodyRow->setVisible(true);
}

void syn::UIWindow::draw(NVGcontext* a_nvg) {
	int ds = theme()->mWindowDropShadowSize, cr = theme()->mWindowCornerRadius;
	int hh = theme()->mWindowHeaderHeight;

	/* Draw window */
	nvgBeginPath(a_nvg);
	nvgRoundedRect(a_nvg, 0, 0, m_size[0], m_size[1], cr);

	nvgFillColor(a_nvg, m_focused ? theme()->mWindowFillFocused : theme()->mWindowFillUnfocused);

	nvgFill(a_nvg);

	/* Draw a drop shadow */
	NVGpaint shadowPaint = nvgBoxGradient(a_nvg, 0, 0, m_size[0], m_size[1], cr * 2, ds * 2,
		nvgRGBA(0, 0, 0, 255), nvgRGBA(255, 255, 255, 0));

	nvgBeginPath(a_nvg);
	nvgRect(a_nvg, 0 - ds, 0 - ds, m_size[0] + 2 * ds, m_size[1] + 2 * ds);
	nvgRoundedRect(a_nvg, 0, 0, m_size[0], m_size[1], cr);
	nvgPathWinding(a_nvg, NVG_HOLE);
	nvgFillPaint(a_nvg, shadowPaint);
	nvgFill(a_nvg);

	/* Draw header */
	NVGpaint headerPaint = nvgLinearGradient(
		a_nvg, 0, 0, 0,
		0 + hh,
		theme()->mWindowHeaderGradientTop,
		theme()->mWindowHeaderGradientBot);

	nvgBeginPath(a_nvg);
	nvgRoundedRect(a_nvg, 0, 0, m_size.x(), hh, cr);

	nvgFillPaint(a_nvg, headerPaint);
	nvgFill(a_nvg);

	nvgBeginPath(a_nvg);
	nvgRoundedRect(a_nvg, 0, 0, m_size.x(), hh, cr);
	nvgStrokeColor(a_nvg, theme()->mWindowHeaderSepTop);
	nvgScissor(a_nvg, 0, 0, m_size.x(), 0.5f);
	nvgStroke(a_nvg);
	nvgResetScissor(a_nvg);

	nvgBeginPath(a_nvg);
	nvgMoveTo(a_nvg, 0 + 0.5f, 0 + hh - 1.5f);
	nvgLineTo(a_nvg, 0 + m_size.x() - 0.5f, 0 + hh - 1.5);
	nvgStrokeColor(a_nvg, theme()->mWindowHeaderSepBot);
	nvgStroke(a_nvg);
}

void syn::UIWindow::_onResize() {
	m_col->setSize(size() - m_col->getRelPos());
	m_headerRow->pack();
	m_bodyRow->pack();
}