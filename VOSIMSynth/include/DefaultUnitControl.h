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

#ifndef __DEFAULTUNITCONTROL__
#define __DEFAULTUNITCONTROL__

#include "UnitControl.h"

namespace syn {
	class DefaultUnitControl : public UnitControl
	{
	public:
		DefaultUnitControl() :
			UnitControl(),
			m_lastSelectedParam(-1)
		{}

		void draw(IGraphics* a_graphics) override;

		void onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod) override;

		void onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod) override;

		void onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod) override;

		void onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod) override;

		void onMouseOver(int a_x, int a_y, IMouseMod* a_mouseMod) override;

		void onMouseWheel(int a_x, int a_y, IMouseMod* a_mouseMod, int a_d) override;

		int getSelectedParam(int a_x, int a_y) override;

		bool isHit(int a_x, int a_y) const override;

	protected:

		void onSetUnitId_() override;

		void onChangeRect_() override;
	private:
		DefaultUnitControl(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y);

		UnitControl* _construct(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y) const override;
	private:
		vector<ITextSlider> m_paramControls;
		int m_lastSelectedParam;

		const int c_minParamHeight = 12;
	};

}

#endif