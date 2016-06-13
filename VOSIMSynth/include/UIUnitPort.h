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

/**
 *  \file UIPort.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 05/2016
 */

#ifndef __UIPORT__
#define __UIPORT__
#include "UIComponent.h"

namespace syn
{
	class UIUnitPort : public UIComponent
	{
	public:
		UIUnitPort(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, int a_portNum, bool a_isInput);
		bool onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		UIComponent* onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
		bool onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;

		int getUnitId() const {
			return m_unitId;
		}

		int getPortId() const {
			return m_portNum;
		}

		bool isInput() const {
			return m_isInput;
		}

		Vector2i getVector() const {
			return { getUnitId(), getPortId() };
		}

	protected:
		void draw(NVGcontext* a_nvg) override;
	protected:
		VoiceManager* m_vm;
		int m_unitId;
		int m_portNum;
		bool m_isInput;

		bool m_isDragging = false;

		int m_textHeight = 12;
	};
}
#endif
