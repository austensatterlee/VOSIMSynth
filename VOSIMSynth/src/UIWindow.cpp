#include "UIWindow.h"
#include "UICell.h"
#include "UILabel.h"
#include "Theme.h"

syn::UIWindow::UIWindow(MainWindow* a_window, const string& a_title) :
	UIComponent{ a_window },
	m_isLocked(false) 
{
	m_headerRow = new UIRow(a_window);
	m_headerRow->setPadding({ 0,2,0,2 });
	m_headerRow->setSize({ -1,theme()->mWindowHeaderHeight });
	m_title = new UILabel(a_window);
	m_title->setText(a_title);
	m_title->setFontSize(16.0);
	m_title->setFontColor(theme()->mTextColor);
	m_headerRow->addChild(m_title);
	m_headerRow->setGreedyChild(m_title);
	addChild(m_headerRow);
}

void syn::UIWindow::addChildToHeader(UIComponent* a_newChild) const {
	m_headerRow->addChild(a_newChild);
}

bool syn::UIWindow::onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	if (UIComponent::onMouseDrag(a_relCursor, a_diffCursor))
		return true;
	if(!m_isLocked)
		move(a_diffCursor);
	return true;
}

syn::UIComponent* syn::UIWindow::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* child = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (child)
		return child;

	if (a_relCursor.localCoord(this).y() < theme()->mWindowHeaderHeight) {
		return this;
	}
	return nullptr;
}

void syn::UIWindow::notifyChildResized(UIComponent* a_child) {
	Vector2i minSize = { 0,0 };
	for (shared_ptr<UIComponent> child : m_children) {
		Vector2i minChildExtent = child->getRelPos() + child->minSize();
		minSize = minSize.cwiseMax(minChildExtent);
	}
	setMinSize(minSize);
}

void syn::UIWindow::lockPosition(bool a_lockPosition) {
	m_isLocked = a_lockPosition;
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
	nvgSave(a_nvg);
	nvgIntersectScissor(a_nvg, 0, 0, m_size.x(), 0.5f);
	nvgStroke(a_nvg);
	nvgRestore(a_nvg);

	nvgBeginPath(a_nvg);
	nvgMoveTo(a_nvg, 0 + 0.5f, 0 + hh - 1.5f);
	nvgLineTo(a_nvg, 0 + m_size.x() - 0.5f, 0 + hh - 1.5);
	nvgStrokeColor(a_nvg, theme()->mWindowHeaderSepBot);
	nvgStroke(a_nvg);
}

void syn::UIWindow::_onResize() {
	m_headerRow->setSize({ size().x(),-1 });
}