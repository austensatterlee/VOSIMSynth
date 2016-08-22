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
 *  \file UIScrollPanel.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 07/2016
 */

#ifndef __UISCROLLPANEL__
#define __UISCROLLPANEL__
#include "UIComponent.h"

namespace synui {
	class UIScrollPanel : public UIComponent
	{
	public:
		UIScrollPanel(MainWindow* a_window);

		void scroll(const Vector2i& a_amt);;
		void setScrollPos(const Vector2i& a_pos);

		Vector2i getScrollOffset() const;

		void notifyChildResized(UIComponent* a_child) override;
		bool onMouseScroll(const UICoord& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) override;
	protected:
		void setChildrenStyles(NVGcontext* a_nvg) override;

		void updateExtents_();

		void updateChildPosition_(shared_ptr<UIComponent> a_child);

		Vector2i scrollBarPos() const;
		Vector2i scrollBarSize() const;

	private:
		void _onAddChild(shared_ptr<UIComponent> a_newchild) override;

		void _onRemoveChild() override;

		void _onChildMoved(UIComponent* a_child) override;

		void _onResize() override;
	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		Vector2i m_minExtent;
		Vector2i m_maxExtent;
		Vector2i m_scrollOffset;
		std::map<UIComponent*, Vector2i> m_originalPositions;
		std::map<UIComponent*, bool> m_dirtyMap;

		float m_scrollBarWidth = 10.0f;
	};
}
#endif
