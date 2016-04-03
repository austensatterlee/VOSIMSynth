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

#include "UnitControl.h"
#include <DSPMath.h>

namespace syn
{
	bool UnitControl::isHit(int a_x, int a_y) const {
		return m_pos[0] < a_x && m_pos[1] < a_y && m_pos[0] + m_size[0] > a_x && m_pos[1] + m_size[1] > a_y;
	}

	void UnitControl::changeRect(NDPoint<2, int> a_newPos, NDPoint<2, int> a_newSize) {
		m_pos = a_newPos;
		m_size[0] = MAX(m_minSize[0], a_newSize[0]);
		m_size[1] = MAX(m_minSize[1], a_newSize[1]);
		onChangeRect_();
	}

	NDPoint<2, int> UnitControl::getMinSize() const { return m_minSize; }

	void UnitControl::setUnitId(int a_newUnitId) { m_unitId = a_newUnitId; onSetUnitId_(); }

	void UnitControl::updateMinSize_(NDPoint<2, int> a_newMinSize) {
		m_minSize[0] = MAX(m_minSize[0], a_newMinSize[0]);
		m_minSize[1] = MAX(m_minSize[1], a_newMinSize[1]);
	}

	void UnitControl::resetMinSize_() {
		m_minSize = { 0,0 };
	}

	int UnitControl::getSelectedParam(int a_x, int a_y) { return -1; }
}

