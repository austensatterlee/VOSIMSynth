#include "UILabel.h"

syn::UILabel::UILabel(VOSIMWindow* a_window): 
	UIComponent(a_window),
	m_fontSize(theme()->mStandardFontSize),
	m_align(NVG_ALIGN_LEFT|NVG_ALIGN_TOP)
{}

void syn::UILabel::setText(const string& a_newText) {
	m_text = a_newText;
	setMinSize_(_measureText());
}

void syn::UILabel::setFontSize(float a_newFontSize) {
	m_fontSize = a_newFontSize;
	setMinSize_( _measureText() );
}

void syn::UILabel::setFontColor(const Color& a_newColor) {
	m_color = a_newColor;
}

void syn::UILabel::setAlignment(int a_newAlign) {
	m_align = a_newAlign;
}

const string& syn::UILabel::text() const { return m_text; }

float syn::UILabel::fontSize() const { return m_fontSize; }

const syn::Color& syn::UILabel::fontColor() const { return m_color; }

int syn::UILabel::alignment() const {
	return m_align;
}

syn::UIComponent* syn::UILabel::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	return nullptr;
}

void syn::UILabel::draw(NVGcontext* a_nvg) {
	nvgFontSize(a_nvg, m_fontSize);
	nvgFillColor(a_nvg, m_color);
	nvgTextAlign(a_nvg, m_align);
	nvgText(a_nvg, 0, 0, m_text.c_str(), nullptr);
}

Eigen::Vector2i syn::UILabel::_measureText() const {
	float bounds[4];
	NVGcontext* nvg = m_window->getContext();
	nvgSave(nvg);
	nvgFontSize(nvg, m_fontSize);	
	nvgTextBounds(nvg, 0, 0, m_text.c_str(), nullptr, bounds);
	nvgRestore(nvg);
	Vector2i textSize{ bounds[2] - bounds[0],bounds[3] - bounds[1] };
	return textSize;
}
