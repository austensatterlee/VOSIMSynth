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
#include "nanovg.h"
#include <UIComponent.h>

namespace syn {
	class UIWindow : public UIComponent
	{
	public:

		UIWindow(VOSIMWindow* a_window, const string& a_title = "Untitled")
			: UIComponent{ a_window },
			m_title{ a_title },
			m_isCollapsed(false)
		{}

		~UIWindow() override
		{};

		bool onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;

		bool onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;

		void toggleCollapsed();
		void collapse();
		void expand();
		Vector2i calcAutoSize() const override;
	protected:
		void draw(NVGcontext* a_nvg) override;	
	protected:
		string m_title;
		bool m_isCollapsed;
		Vector2i m_oldSize;
		int m_autoWidth;
	};
}
#endif