#include "UILabel.h"
#include "Theme.h"

syn::UILabel::UILabel(VOSIMWindow* a_window):
	UIComponent(a_window),
	m_fontFace(theme()->mFontNormal), 
	m_color(1.0f,1.0f,1.0f,1.0f),
	m_fontSize(theme()->mStandardFontSize),
	m_fontBlur(0),
	m_align(NVG_ALIGN_LEFT | NVG_ALIGN_TOP)
{}

void syn::UILabel::setText(const string& a_newText) {
	m_text = a_newText;
	setMinSize(_measureText());
}

void syn::UILabel::setFontSize(float a_newFontSize) {
	m_fontSize = a_newFontSize;
	setMinSize(_measureText());
}

void syn::UILabel::setFontBlur(float a_fontBlur) {
	m_fontBlur = a_fontBlur;
}

float syn::UILabel::fontBlur() const {
	return m_fontBlur;
}

int syn::UILabel::fontFace() const {
	return m_fontFace;
}

void syn::UILabel::setFontColor(const Color& a_newColor) {
	m_color = a_newColor;
}

void syn::UILabel::setFontFace(int a_fontFace) {
	m_fontFace = a_fontFace;
	setMinSize(_measureText());
}

void syn::UILabel::setAlignment(int a_newAlign) {
	m_align = a_newAlign;
}

const string& syn::UILabel::text() const {
	return m_text;
}

float syn::UILabel::fontSize() const {
	return m_fontSize;
}

const syn::Color& syn::UILabel::fontColor() const {
	return m_color;
}

int syn::UILabel::alignment() const {
	return m_align;
}

syn::UIComponent* syn::UILabel::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	return nullptr;
}

void syn::UILabel::draw(NVGcontext* a_nvg) {
	nvgFontFaceId(a_nvg, m_fontFace);
	nvgFontBlur(a_nvg, m_fontBlur);	
	nvgFontSize(a_nvg, m_fontSize);
	nvgFillColor(a_nvg, m_color);
	nvgTextAlign(a_nvg, m_align);
	Vector2i pos;
	switch(m_align&0x7) {
	case NVG_ALIGN_RIGHT:
		pos[0] = size()[0];
		break;
	case NVG_ALIGN_CENTER:
		pos[0] = size()[0] / 2;
		break;
	case NVG_ALIGN_LEFT:
	default:
		pos[0] = 0;
		break;
	}
	switch (m_align & 0x78) {
	case NVG_ALIGN_TOP:
		pos[1] = 0;
		break;
	case NVG_ALIGN_MIDDLE:
		pos[1] = size()[1] / 2;
		break;
	case NVG_ALIGN_BOTTOM:
	case NVG_ALIGN_BASELINE:
	default:
		pos[1] = size()[1];
		break;
	}
	nvgText(a_nvg, pos[0], pos[1], m_text.c_str(), nullptr);
}

Eigen::Vector2i syn::UILabel::_measureText() const {
	float bounds[4];
	NVGcontext* nvg = m_window->getContext();
	nvgSave(nvg);
	nvgFontSize(nvg, m_fontSize);
	nvgTextBounds(nvg, 0, 0, m_text.c_str(), nullptr, bounds);
	nvgRestore(nvg);
	Vector2i textSize{bounds[2] - bounds[0],bounds[3] - bounds[1]};
	return textSize;
}
