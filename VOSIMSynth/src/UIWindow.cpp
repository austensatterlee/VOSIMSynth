#include "UIWindow.h"
#include "VosimWindow.h"

bool syn::UIWindow::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	move(a_diffCursor);
	return true;
}

bool syn::UIWindow::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {

	if (a_relCursor.y() - m_pos.y() < m_theme->mWindowHeaderHeight) {
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
	setSize({ m_size[0],m_theme->mWindowHeaderHeight });
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

Eigen::Vector2i syn::UIWindow::calcAutoSize() const {
	Vector2i size = m_size;	
	size[0] = MAX(size[0], m_autoWidth);
	for(shared_ptr<UIComponent> child : m_children) {
		if (child->visible()) {
			size = size.cwiseMax(child->size()+Vector2i{0,m_theme->mWindowHeaderHeight});
		}
	}
	return size;
}

void syn::UIWindow::draw(NVGcontext* a_nvg) {
	int ds = m_theme->mWindowDropShadowSize, cr = m_theme->mWindowCornerRadius;
	int hh = m_theme->mWindowHeaderHeight;

	/* Draw window */
	nvgSave(a_nvg);
	nvgBeginPath(a_nvg);
	nvgRoundedRect(a_nvg, 0, 0, m_size[0], m_size[1], cr);

	nvgFillColor(a_nvg, m_focused ? m_theme->mWindowFillFocused : m_theme->mWindowFillUnfocused);

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
			m_theme->mWindowHeaderGradientTop,
			m_theme->mWindowHeaderGradientBot);

		nvgBeginPath(a_nvg);
		nvgRoundedRect(a_nvg, 0, 0, m_size.x(), hh, cr);

		nvgFillPaint(a_nvg, headerPaint);
		nvgFill(a_nvg);

		nvgBeginPath(a_nvg);
		nvgRoundedRect(a_nvg, 0, 0, m_size.x(), hh, cr);
		nvgStrokeColor(a_nvg, m_theme->mWindowHeaderSepTop);
		nvgScissor(a_nvg, 0, 0, m_size.x(), 0.5f);
		nvgStroke(a_nvg);
		nvgResetScissor(a_nvg);

		nvgBeginPath(a_nvg);
		nvgMoveTo(a_nvg, 0 + 0.5f, 0 + hh - 1.5f);
		nvgLineTo(a_nvg, 0 + m_size.x() - 0.5f, 0 + hh - 1.5);
		nvgStrokeColor(a_nvg, m_theme->mWindowHeaderSepBot);
		nvgStroke(a_nvg);

		nvgFontSize(a_nvg, 18.0f);
		nvgFontFace(a_nvg, "sans-bold");
		nvgTextAlign(a_nvg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

		nvgFontBlur(a_nvg, 2);
		nvgFillColor(a_nvg, m_theme->mDropShadow);
		nvgText(a_nvg, 0 + m_size.x() / 2,
			0 + hh / 2, m_title.c_str(), nullptr);

		nvgFontBlur(a_nvg, 0);
		nvgFillColor(a_nvg, m_focused ? m_theme->mWindowTitleFocused
			                    : m_theme->mWindowTitleUnfocused);
		m_autoWidth = MAX(m_autoWidth,10+nvgText(a_nvg, 0 + m_size.x() / 2, 0 + hh / 2 - 1,
		        m_title.c_str(), nullptr));
	}

	nvgRestore(a_nvg);
}
