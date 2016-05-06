/*
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
along with VOSIMProject.  If not, see <http://www.gnu.org/licenses/>.

Copyright 2016, Austen Satterlee
*/

/**
*  \file UITextSlider.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UITEXTSLIDER__
#define __UITEXTSLIDER__
#include "UIComponent.h"
#include <VoiceManager.h>

namespace syn
{
	class UITextSlider : public UIComponent
	{	
	public:
		UITextSlider(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, int a_paramId) :
			UIComponent{ a_window },
			m_value(0.0),
			m_vm(a_vm),
			m_unitId(a_unitId),
			m_paramId(a_paramId),
			m_autoWidth(0)
		{}

		bool onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseScroll(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) override;
		Vector2i calcAutoSize(NVGcontext* a_nvg) const override;
	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		double m_value;
		VoiceManager* m_vm;
		int m_unitId;
		int m_paramId;

		int m_autoWidth;
	};
}
#endif