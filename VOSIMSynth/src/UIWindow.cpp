#include "UIWindow.h"

bool syn::UIWindow::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	move(a_diffCursor);
	return true;
}

bool syn::UIWindow::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	if (a_relCursor.y() - m_pos.y() < m_theme->mWindowHeaderHeight)
		return true;
	return false;
}

void syn::UIWindow::draw(NVGcontext* a_nvg) {
	int ds = m_theme->mWindowDropShadowSize, cr = m_theme->mWindowCornerRadius;
	int hh = m_theme->mWindowHeaderHeight;

	/* Draw window */
	nvgSave(a_nvg);
	nvgBeginPath(a_nvg);
	nvgRoundedRect(a_nvg, m_pos[0], m_pos[1], m_size[0], m_size[1], cr);

	nvgFillColor(a_nvg, m_focused ? m_theme->mWindowFillFocused : m_theme->mWindowFillUnfocused);

	nvgFill(a_nvg);

	/* Draw a drop shadow */
	NVGpaint shadowPaint = nvgBoxGradient(a_nvg, m_pos[0], m_pos[1], m_size[0], m_size[1], cr * 2, ds * 2,
	                                      nvgRGBA(0, 0, 0, 255), nvgRGBA(255, 255, 255, 0));

	nvgBeginPath(a_nvg);
	nvgRect(a_nvg, m_pos[0] - ds, m_pos[1] - ds, m_size[0] + 2 * ds, m_size[1] + 2 * ds);
	nvgRoundedRect(a_nvg, m_pos[0], m_pos[1], m_size[0], m_size[1], cr);
	nvgPathWinding(a_nvg, NVG_HOLE);
	nvgFillPaint(a_nvg, shadowPaint);
	nvgFill(a_nvg);

	nvgFontFace(a_nvg, "sans");

	if (!m_title.empty()) {
		/* Draw header */
		NVGpaint headerPaint = nvgLinearGradient(
			a_nvg, m_pos.x(), m_pos.y(), m_pos.x(),
			m_pos.y() + hh,
			m_theme->mWindowHeaderGradientTop,
			m_theme->mWindowHeaderGradientBot);

		nvgBeginPath(a_nvg);
		nvgRoundedRect(a_nvg, m_pos.x(), m_pos.y(), m_size.x(), hh, cr);

		nvgFillPaint(a_nvg, headerPaint);
		nvgFill(a_nvg);

		nvgBeginPath(a_nvg);
		nvgRoundedRect(a_nvg, m_pos.x(), m_pos.y(), m_size.x(), hh, cr);
		nvgStrokeColor(a_nvg, m_theme->mWindowHeaderSepTop);
		nvgScissor(a_nvg, m_pos.x(), m_pos.y(), m_size.x(), 0.5f);
		nvgStroke(a_nvg);
		nvgResetScissor(a_nvg);

		nvgBeginPath(a_nvg);
		nvgMoveTo(a_nvg, m_pos.x() + 0.5f, m_pos.y() + hh - 1.5f);
		nvgLineTo(a_nvg, m_pos.x() + m_size.x() - 0.5f, m_pos.y() + hh - 1.5);
		nvgStrokeColor(a_nvg, m_theme->mWindowHeaderSepBot);
		nvgStroke(a_nvg);

		nvgFontSize(a_nvg, 18.0f);
		nvgFontFace(a_nvg, "sans-bold");
		nvgTextAlign(a_nvg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

		nvgFontBlur(a_nvg, 2);
		nvgFillColor(a_nvg, m_theme->mDropShadow);
		nvgText(a_nvg, m_pos.x() + m_size.x() / 2,
		        m_pos.y() + hh / 2, m_title.c_str(), nullptr);

		nvgFontBlur(a_nvg, 0);
		nvgFillColor(a_nvg, m_focused ? m_theme->mWindowTitleFocused
			                    : m_theme->mWindowTitleUnfocused);
		nvgText(a_nvg, m_pos.x() + m_size.x() / 2, m_pos.y() + hh / 2 - 1,
		        m_title.c_str(), nullptr);
	}

	nvgRestore(a_nvg);
}
