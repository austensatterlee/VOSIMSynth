/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

#include "DefaultUnitControl.h"
#include <DSPMath.h>

namespace syn
{
	void DefaultUnitControl::draw(IGraphics* a_graphics) {
		int nParams = m_paramControls.size();
		resetMinSize_();
		// Draw user controls
		for (int i = 0; i < nParams; i++) {
			m_paramControls[i].Draw(a_graphics);
			updateMinSize_({ m_paramControls[i].getMinSize(), 0 });
		}
		updateMinSize_({ 0, c_minParamHeight*nParams });
	}

	void DefaultUnitControl::onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod) {
		int selectedParam = getSelectedParam(a_x, a_y);
		if (selectedParam >= 0) {
			m_paramControls[selectedParam].OnMouseDblClick(a_x, a_y, a_mouseMod);
		}
	}

	void DefaultUnitControl::onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod) {
		m_lastSelectedParam = getSelectedParam(a_x, a_y);
		if (m_lastSelectedParam >= 0) {
			m_paramControls[m_lastSelectedParam].OnMouseDown(a_x, a_y, a_mouseMod);
		}
	}

	void DefaultUnitControl::onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod) {
		if (m_lastSelectedParam >= 0) {
			m_paramControls[m_lastSelectedParam].OnMouseUp(a_x, a_y, a_mouseMod);
		}
		m_lastSelectedParam = -1;
	}

	void DefaultUnitControl::onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod) {
		if (m_lastSelectedParam >= 0) {
			m_paramControls[m_lastSelectedParam].OnMouseDrag(a_x, a_y, a_dX, a_dY, a_mouseMod);
		}
	}

	void DefaultUnitControl::onMouseOver(int a_x, int a_y, IMouseMod* a_mouseMod) {
		int selectedParam = getSelectedParam(a_x, a_y);
		if (selectedParam >= 0) {
			m_paramControls[selectedParam].OnMouseOver(a_x, a_y, a_mouseMod);
		}
	}

	void DefaultUnitControl::onMouseWheel(int a_x, int a_y, IMouseMod* a_mouseMod, int a_d) {
		int selectedParam = getSelectedParam(a_x, a_y);
		if (selectedParam >= 0) {
			m_paramControls[selectedParam].OnMouseWheel(a_x, a_y, a_mouseMod, a_d);
		}
	}

	int DefaultUnitControl::getSelectedParam(int a_x, int a_y) {
		int selectedParam = -1;
		for (int i = 0; i < m_paramControls.size(); i++) {
			if (m_paramControls[i].GetRECT()->Contains(a_x, a_y)) {
				selectedParam = i;
			}
		}
		return selectedParam;
	}

	bool DefaultUnitControl::isHit(int a_x, int a_y) const {
		int nParams = m_paramControls.size();
		if (!nParams)
			return false;
		return UnitControl::isHit(a_x, a_y);
	}

	void DefaultUnitControl::onSetUnitId_() {
		m_paramControls.clear();
		int nParams = m_voiceManager->getUnit(m_unitId).getNumParameters();
		for (int i = 0; i < nParams; i++) {
			m_paramControls.push_back(ITextSlider(m_plug, m_voiceManager, m_unitId, i, IRECT{ 0, 0, 0, 0 }));
		}
	}

	void DefaultUnitControl::onChangeRect_() {
		int nParams = m_paramControls.size();
		if (nParams) {
			int portY = m_pos[1];
			int rowsize = MAX(c_minParamHeight,m_size[1] / nParams);
			for (int i = 0; i < nParams; i++) {
				IRECT param_rect{ m_pos[0], portY, m_pos[0] + m_size[0], portY + rowsize };
				m_paramControls[i].setRECT(param_rect);
				portY += rowsize;
			}
		}
	}

	DefaultUnitControl::DefaultUnitControl(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y):
		UnitControl(a_plug, a_vm, a_unitId, a_x, a_y),
		m_lastSelectedParam(-1) {
		int nParams = m_voiceManager->getUnit(m_unitId).getNumParameters();
		m_paramControls.clear();
		for (int i = 0; i < nParams; i++) {
			m_paramControls.push_back(ITextSlider(m_plug, m_voiceManager, m_unitId, i, IRECT{ 0, 0, 0, 0 }));
		}
	}

	UnitControl* DefaultUnitControl::_construct(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y) const {
		return new DefaultUnitControl(a_plug, a_vm, a_unitId, a_x, a_y);
	}
}

