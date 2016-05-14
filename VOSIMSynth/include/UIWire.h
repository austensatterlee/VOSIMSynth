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
 *  \file UIWire.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 05/2016
 */

#ifndef __UIWIRE__
#define __UIWIRE__
#include "UIComponent.h"

namespace syn
{
	class UIWire : public UIComponent
	{
	public:
		struct compareByHoverDistance {
			bool operator()(UIWire* a_lhs, UIWire* a_rhs) { return a_lhs->hoverDistance() < a_rhs->hoverDistance(); }
		};
	public:
		UIWire(VOSIMWindow* a_window, int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort)
			: UIComponent{a_window}, m_fromUnit(a_fromUnit), m_fromPort(a_fromPort), m_toUnit(a_toUnit), m_toPort(a_toPort), m_hoverDistance(INFINITY) {}

		bool contains(const Vector2i& a_pt) override;
		void onMouseEnter(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isEntering) override;
		bool onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		UIComponent* onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;

		double hoverDistance() const {
			return hovered() ? m_hoverDistance : INFINITY;
		}

		int fromUnit() const {
			return m_fromUnit;
		}

		int fromPort() const {
			return m_fromPort;
		}

		int toUnit() const {
			return m_toUnit;
		}

		int toPort() const {
			return m_toPort;
		}

		Vector2i fromPt() const;
		Vector2i toPt() const;
	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		int m_fromUnit, m_fromPort, m_toUnit, m_toPort;
		bool m_isDragging = false;
		bool m_isDraggingInput = false;
		double m_hoverDistance;
	};
}
#endif
