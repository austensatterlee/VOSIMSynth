#include "UIWindow.h"
#include "VosimWindow.h"

bool syn::UIWindow::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	if (UIComponent::onMouseDrag(a_relCursor, a_diffCursor))
		return true;
	move(a_diffCursor);
	return true;
}

bool syn::UIWindow::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	if (UIComponent::onMouseDown(a_relCursor, a_diffCursor))
		return true;
	if (a_relCursor.y() - m_pos.y() < theme()->mWindowHeaderHeight) {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			return true;
		} else if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
			toggleCollapsed();
			return true;
		}
	}
	return false;
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
	setSize({ m_size[0],theme()->mWindowHeaderHeight });
	for(int i=0;i<m_children.size();i++) {
		m_children[i]->setVisible(false);
	}
}

void syn::UIWindow::expand() {
	m_isCollapsed = false;
	setSize(m_oldSize);
	for (int i = 0; i<m_children.size(); i++) {
		m_children[i]->setVisible(true);
	}
}

Eigen::Vector2i syn::UIWindow::calcAutoSize(NVGcontext* a_nvg) const {
	Vector2i autoSize{ m_titleWidth,theme()->mWindowHeaderHeight };
	for(shared_ptr<UIComponent> child : m_children) {
		if (child->visible()) {
			autoSize = autoSize.cwiseMax(child->size()+child->getRelPos()+Vector2i::Ones()*5);
		}
	}
	return autoSize;
}

void syn::UIWindow::draw(NVGcontext* a_nvg) {
	int ds = theme()->mWindowDropShadowSize, cr = theme()->mWindowCornerRadius;
	int hh = theme()->mWindowHeaderHeight;

	/* Draw window */
	nvgSave(a_nvg);
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

	nvgFontFace(a_nvg, "sans");

	if (!m_title.empty()) {
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

		nvgFontSize(a_nvg, 16.0f);
		nvgFontFace(a_nvg, "sans-bold");
		nvgTextAlign(a_nvg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

		nvgFontBlur(a_nvg, 1);
		nvgFillColor(a_nvg, theme()->mDropShadow);
		nvgText(a_nvg, 2, 0 + hh / 2, m_title.c_str(), nullptr);

		nvgFontBlur(a_nvg, 0);
		nvgFillColor(a_nvg, m_focused ? theme()->mWindowTitleFocused
			                    : theme()->mWindowTitleUnfocused);
		nvgText(a_nvg, 2, 0 + hh / 2 - 1, m_title.c_str(), nullptr);

		// Calculate title width
		float bounds[4];
		nvgSave(a_nvg);
		nvgTextAlign(a_nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgTextBounds(a_nvg, 0, 0, m_title.c_str(), nullptr, bounds);
		nvgRestore(a_nvg);
		m_titleWidth = 2 + bounds[2] - bounds[0];
	}

	nvgRestore(a_nvg);
}
