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
 *  \file UIComponent.h
 *  \brief  
 *  \details
 *  \author Austen Satterlee
 *  \date 04/2016
 */

#ifndef __UICOMPONENT__
#define __UICOMPONENT__

#include <SFML/Window.hpp>

#include <thread>
#include <NDPoint.h>

#include "nanovg.h"
#include <Theme.h>
#include "VOSIMWindow.h"

namespace syn
{

	class UIComponent
	{
		friend class VOSIMWindow;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		UIComponent(VOSIMWindow* a_window);

		virtual ~UIComponent() {}

		void recursiveDraw(NVGcontext* a_nvg);
		
		void addChild(UIComponent* a_newChild);

		bool removeChild(UIComponent* a_child);
		bool removeChild(int a_index);

		shared_ptr<UIComponent> getChild(int i);

		int numChildren() const {return m_children.size();}

		UIComponent* parent() const {return m_parent;}

		virtual bool contains(const Vector2i& a_pt);

		UIComponent* findChild(const Vector2i& a_pt);

		Vector2i getRelPos() const {return m_pos;}

		void setRelPos(const Vector2i& a_pos) {m_pos = a_pos;}
		
		Vector2i getAbsPos() const {return m_parent ? m_parent->getAbsPos() + getRelPos() : getRelPos();}
		Vector2i getAbsCenter() const { return getAbsPos() + size() / 2.0; }

		void move(const Vector2i& a_dist) {m_pos += a_dist;}

		Vector2i size() const { return m_size; }

		void setSize(const Vector2i& a_size) { m_size = a_size; onResize(); }

		void grow(const Vector2i& a_amt) { m_size += a_amt; onResize(); }

		shared_ptr<Theme> theme() const {return m_window->theme();}

		bool visible() const {return m_visible;}

		void setVisible(bool a_visible) {m_visible = a_visible;}

		void setAutosize(bool a_autosize) { m_autoSize = a_autosize; }

		virtual Vector2i calcAutoSize() const;
		virtual bool onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor);
		virtual bool onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor);
		virtual bool onMouseEnter(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isEntering);
		virtual bool onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor);
		virtual bool onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor);
		virtual bool onMouseScroll(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt);
		virtual void onResize() {};
		virtual void onFocusEvent(bool a_isFocused);


	protected:
		virtual void draw(NVGcontext* a_nvg) {};

	protected:
		UIComponent* m_parent;
		VOSIMWindow* m_window;
		vector<shared_ptr<UIComponent>> m_children;

		bool m_visible, m_focused, m_autoSize;
		Vector2i m_pos, m_size;
	};
};
#endif

