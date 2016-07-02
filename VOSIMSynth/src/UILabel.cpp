#include "UILabel.h"
#include "Theme.h"

synui::UILabel::UILabel(MainWindow* a_window):
	UIComponent(a_window),
	m_fontFace(theme()->mFontNormal), 
	m_color(1.0f,1.0f,1.0f,1.0f),
	m_fontSize(theme()->mStandardFontSize),
	m_fontBlur(0),
	m_align(NVG_ALIGN_LEFT | NVG_ALIGN_TOP) 
{
	_updateMinSize();	
}

void synui::UILabel::setText(const string& a_newText) {
	m_text = a_newText;
	_updateMinSize();
}

void synui::UILabel::setFontSize(float a_newFontSize) {
	m_fontSize = a_newFontSize;
	_updateMinSize();
}

void synui::UILabel::setFontBlur(float a_fontBlur) {
	m_fontBlur = a_fontBlur;
}

float synui::UILabel::fontBlur() const {
	return m_fontBlur;
}

int synui::UILabel::fontFace() const {
	return m_fontFace;
}

void synui::UILabel::setFontColor(const Color& a_newColor) {
	m_color = a_newColor;
}

void synui::UILabel::setFontFace(int a_fontFace) {
	m_fontFace = a_fontFace;
	_updateMinSize();
}

void synui::UILabel::setAlignment(int a_newAlign) {
	m_align = a_newAlign;
}

const string& synui::UILabel::text() const {
	return m_text;
}

float synui::UILabel::fontSize() const {
	return m_fontSize;
}

const synui::Color& synui::UILabel::fontColor() const {
	return m_color;
}

int synui::UILabel::alignment() const {
	return m_align;
}

synui::UIComponent* synui::UILabel::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	return nullptr;
}

void synui::UILabel::draw(NVGcontext* a_nvg) {
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

void synui::UILabel::_updateMinSize() {
	float bounds[4];
	NVGcontext* nvg = m_window->getContext();
	nvgSave(nvg);
	nvgFontSize(nvg, m_fontSize);
	float width = nvgTextBounds(nvg, 0, 0, m_text.c_str(), nullptr, bounds);
	nvgRestore(nvg);
	Vector2i textSize{width, bounds[3] - bounds[1]};
	setMinSize(textSize);
}
