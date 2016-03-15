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

#ifndef __UNITCONTROL__
#define __UNITCONTROL__

#include "IControl.h"
#include "NDPoint.h"
#include "ITextSlider.h"

namespace syn {

	class UnitControl
	{
	public:
		UnitControl();

		/// This method should be used to create new UnitControl instances
		UnitControl* construct(IPlugBase* a_mPlug, shared_ptr<VoiceManager> a_voiceManager, int a_uid, int a_x, int a_y) const;

		virtual ~UnitControl() {}

		virtual void draw(IGraphics* a_graphics) = 0;

		virtual void onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual void onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual void onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual void onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod) {};

		virtual void onMouseWheel(int a_x, int a_y, IMouseMod* a_mouseMod, int a_d) {};

		virtual void onMouseOver(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual bool isHit(int a_x, int a_y) const;

		void move(NDPoint<2, int> a_newPos);

		void resize(NDPoint<2, int> a_newSize);

		NDPoint<2, int> getMinSize() const;

		void setUnitId(int a_newUnitId);

		virtual int getSelectedParam(int a_x, int a_y);
	protected:
		UnitControl(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y);

		void updateMinSize_(NDPoint<2, int> a_newMinSize);

		void resetMinSize_();

		virtual void onSetUnitId_() {};
		virtual void onChangeRect_() {};
	protected:
		shared_ptr<VoiceManager> m_voiceManager;
		int m_unitId;
		NDPoint<2, int> m_pos;
		NDPoint<2, int> m_size;
		IPlugBase* m_plug;
	private:
		virtual UnitControl* _construct(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y) const = 0;
	private:
		NDPoint<2, int> m_minSize;
	};
}

#endif