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
#include <type_traits>

namespace syn {

	class UnitControl
	{
	public:
		UnitControl(IPlugBase* a_plug, VoiceManager* a_voiceManager) :
			m_voiceManager(a_voiceManager),
			m_unitId(-1),
			m_plug(a_plug)
		{}

		/// This method should be used to create new UnitControl instances
		template<typename T>
		static typename enable_if< is_base_of<UnitControl, T>::value, T* >::type
		construct(IPlugBase* a_plug, VoiceManager* a_voiceManager, int a_uid);

		virtual ~UnitControl() {}

		virtual void draw(IGraphics* a_graphics) = 0;

		virtual void onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual void onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual void onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual void onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod) {};

		virtual void onMouseWheel(int a_x, int a_y, IMouseMod* a_mouseMod, int a_d) {};

		virtual void onMouseOver(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual bool isHit(int a_x, int a_y) const;

		void changeRect(NDPoint<2, int> a_newPos, NDPoint<2, int> a_newSize);

		NDPoint<2, int> getMinSize() const;

		void setUnitId(int a_newUnitId);

		virtual int getSelectedParam(int a_x, int a_y);
	protected:
		void updateMinSize_(NDPoint<2, int> a_newMinSize);

		void resetMinSize_();

		virtual void onSetUnitId_() {};
		virtual void onChangeRect_() {};
	protected:
		VoiceManager* m_voiceManager;
		int m_unitId;
		NDPoint<2, int> m_pos;
		NDPoint<2, int> m_size;
		IPlugBase* m_plug;
	private:
		NDPoint<2, int> m_minSize;
	};

	template <typename T>
	typename std::enable_if<std::is_base_of<UnitControl, T>::value, T*>::type 
	UnitControl::construct(IPlugBase* a_plug, VoiceManager* a_voiceManager, int a_uid) {
		UnitControl* ret = new T(a_plug, a_voiceManager);
		ret->setUnitId(a_uid);
		return static_cast<T*>(ret);
	}
}

#endif