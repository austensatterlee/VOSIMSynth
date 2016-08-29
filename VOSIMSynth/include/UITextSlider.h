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
#include "UIParamControl.h"

namespace synui
{
	class UITextSlider : public UIParamControl
	{
	public:
		UITextSlider(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId, int a_paramId);

		bool onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;
		UIComponent* onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
		bool onMouseScroll(const UICoord& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) override;
		
	protected:
		void draw(NVGcontext* a_nvg) override;
		void updateValue_() override;
	private:
		void _onResize() override;
		void _updateMinSize();
	private:
		UIRow* m_row;
		UILabel *m_nameLabel, *m_valueLabel, *m_unitsLabel;
	};
}
#endif
