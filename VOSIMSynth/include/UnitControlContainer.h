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
 *	 \file UnitControlContainer.h
 *   \brief
 *   \details
 *   \author Austen Satterlee
 *   \date 02/15/2016
 */
#ifndef __UNITCONTROLCONTAINER__
#define __UNITCONTROLCONTAINER__
#include "UnitControl.h"
namespace syn {

	struct UnitPortVector {
		enum {
			Null = 0,
			Input,
			Output
		} type;
		int id;
	};

	class UnitControlContainer {
	public:
		UnitControlContainer(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, UnitControl* a_unitControl, int a_unitId, int x, int y);

		virtual ~UnitControlContainer() {
			delete m_unitControl;
		};

		virtual bool draw(IGraphics* pGraphics);

		virtual void onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod);

		/**
		 * \returns true if the event was propogated to the contained UnitControl
		 */
		virtual bool onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod);

		/**
		* \returns true if the event was propogated to the contained UnitControl
		*/
		virtual bool onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod);

		virtual void onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod);

		virtual void onMouseOver(int a_x, int a_y, IMouseMod* a_mouseMod);

		virtual void onMouseWheel(int a_x, int a_y, IMouseMod* a_mouseMod, int a_d);

		virtual void move(int a_newx, int a_newy);

		virtual NDPoint<2, int> getMinSize() const;

		virtual void resize(NDPoint<2, int> a_newSize);

		virtual NDPoint<2, int> getPortPos(UnitPortVector a_portId) const;

		virtual IRECT getPortRect(UnitPortVector a_portVector) const;

		NDPoint<2, int> getPos() const;

		NDPoint<2, int> getSize() const;

		UnitPortVector getSelectedPort(int x, int y) const;

		virtual void setUnitId(int a_newUnitId);

		int getUnitId() const;

		void setIsSelected(bool a_isSelected);

		bool isHit(int a_x, int a_y);

		bool isHit(IRECT& a_rect);
	private:
		virtual void _resetMinSize();
		void _updateMinSize(NDPoint<2, int> a_newMinSize);
	private:
		friend class CircuitPanel;

		shared_ptr<VoiceManager> m_voiceManager;
		UnitControl* m_unitControl;

		int m_unitId;
		bool m_isSelected;
		NDPoint<2, int> m_size;
		NDPoint<2, int> m_minSize;
		NDPoint<2, int> m_portSize;
		NDPoint<2, int> m_titleSize;
		const int c_portPad = 3;
		const int c_edgePad = 1;
		IRECT m_rect;
		IPlugBase* m_plug;
		bool m_isControlClicked;
	};
}
#endif