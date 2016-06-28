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
*  \file UIWindow.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UIWINDOW__
#define __UIWINDOW__
#include "UIComponent.h"

namespace syn
{
	class UIWindow : public UIComponent
	{
	public:

		UIWindow(MainWindow* a_window, const string& a_title = "Untitled");

		void addChildToHeader(UIComponent* a_newChild) const;

		bool onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;

		UIComponent* onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;

		void notifyChildResized(UIComponent* a_child) override;

		void lockPosition(bool a_lockPosition);
	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		void _onResize() override;
	protected:
		UILabel* m_title;
		UICol* m_col;
		UIRow* m_bodyRow;
		UIRow* m_headerRow;
		bool m_isLocked;
	};
}
#endif
