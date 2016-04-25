#include "UIUnitSelector.h"
#include <eigen/src/Core/util/ForwardDeclarations.h>
#include <eigen/src/Core/util/ForwardDeclarations.h>

Eigen::Vector2i syn::UIUnitSelector::calcAutoSize() const {
	return{ m_autoWidth, m_autoHeight };
}

bool syn::UIUnitSelector::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	int row = 0;
	int group = 0;
	set<string> gNames = m_unitFactory->getGroupNames();
	Vector2i cursor = a_relCursor - m_pos;
	for (string gName : gNames) {
		vector<string> pNames = m_unitFactory->getPrototypeNames(gName);
		if(cursor.y()<(row+1)*m_fontSize) {
			if (m_currGroup == group) 
				m_currGroup = -1;
			else {
				m_currGroup = group;
				m_currPrototype = -1;
				m_currPrototypeName = "";
			}
			return true;
		}
		if (group == m_currGroup) {
			for (int j = 0; j < pNames.size(); j++) {
				row++;	
				if (cursor.y()<(row + 1) * m_fontSize) {
					m_currPrototype = j;
					m_currPrototypeName = pNames[j];
					return true;
				}
			}
		}
		row++;
		group++;
	}
	m_currGroup = -1;
	m_currPrototype = -1;
	m_currPrototypeName = "";
	return false;
}

bool syn::UIUnitSelector::onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	int row = 0;
	int group = 0;
	set<string> gNames = m_unitFactory->getGroupNames();
	Vector2i cursor = a_relCursor - m_pos;
	for (string gName : gNames) {
		vector<string> pNames = m_unitFactory->getPrototypeNames(gName);
		if (cursor.y()<(row + 1) * m_fontSize) {
			m_highlightedRow = row;
			return true;
		}
		if (group == m_currGroup) {
			for (int j = 0; j < pNames.size(); j++) {
				row++;
				if (cursor.y()<(row + 1) * m_fontSize) {
					m_highlightedRow = row;
					return true;
				}
			}
		}
		row++;
		group++;
	}
	return false;
}

bool syn::UIUnitSelector::onMouseEnter(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isEntering) {
	if(!a_isEntering)
		m_highlightedRow = -1;
	return true;
}

void syn::UIUnitSelector::draw(NVGcontext* a_nvg) {
	nvgSave(a_nvg);

	nvgFontSize(a_nvg, m_fontSize);
	nvgTextAlign(a_nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

	nvgFontBlur(a_nvg, 0.1);

	int row = 0;
	int group = 0;
	set<string> gNames = m_unitFactory->getGroupNames();
	for(string gName : gNames){
		vector<string> pNames = m_unitFactory->getPrototypeNames(gName);
		nvgFontFace(a_nvg, "sans-bold");
		if (m_highlightedRow==row)
			nvgFillColor(a_nvg, theme()->mHighlightedTextColor);
		else
			nvgFillColor(a_nvg, theme()->mTextColor);
		m_autoWidth = MAX(m_autoWidth, nvgText(a_nvg, 0, row * m_fontSize, gName.c_str(), nullptr));
		if (group == m_currGroup) {
			nvgFontFace(a_nvg, "sans");
			for (int j = 0; j < pNames.size(); j++) {
				row++;
				if (m_highlightedRow==row || m_currPrototype==j)
					nvgFillColor(a_nvg, theme()->mHighlightedTextColor);
				else
					nvgFillColor(a_nvg, theme()->mTextColor);
				m_autoWidth = MAX(m_autoWidth, m_indentAmt+nvgText(a_nvg, m_indentAmt, row * m_fontSize, pNames[j].c_str(), nullptr));
			}
		}
		row++;
		group++;
	}
	m_autoHeight = row * m_fontSize;
	nvgRestore(a_nvg);
}
