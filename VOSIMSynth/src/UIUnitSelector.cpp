#include "UIUnitSelector.h"

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
			else
				m_currGroup = group;
			return true;
		}
		if (group == m_currGroup) {
			for (int j = 0; j < pNames.size(); j++) {
				row++;	
				if (cursor.y()<(row + 1) * m_fontSize) {
					m_currPrototype = j;
					return true;
				}
			}
		}
		row++;
		group++;
	}
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
			nvgFillColor(a_nvg, m_theme->mHighlightedTextColor);
		else
			nvgFillColor(a_nvg, m_theme->mTextColor);
		m_autoWidth = MAX(m_autoWidth, nvgText(a_nvg, 0, row * m_fontSize, gName.c_str(), nullptr));
		if (group == m_currGroup) {
			nvgFontFace(a_nvg, "sans");
			for (int j = 0; j < pNames.size(); j++) {
				row++;
				if (m_highlightedRow==row)
					nvgFillColor(a_nvg, m_theme->mHighlightedTextColor);
				else
					nvgFillColor(a_nvg, m_theme->mTextColor);
				m_autoWidth = MAX(m_autoWidth, nvgText(a_nvg, 0, row * m_fontSize, pNames[j].c_str(), nullptr));
			}
		}
		row++;
		group++;
	}
	m_autoHeight = row * m_fontSize;
	nvgRestore(a_nvg);
}
