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

#include "MainWindow.h"
#include <list>

using std::list;

namespace synui
{
	class UICoord;

	class UIComponent : std::enable_shared_from_this<UIComponent>
	{
		friend class MainWindow;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		UIComponent(MainWindow* a_window);

		virtual ~UIComponent() {}

		void recursiveDraw(NVGcontext* a_nvg);

		void addChild(UIComponent* a_newChild, const string& a_group="");
		void addChild(shared_ptr<UIComponent> a_newChild, const string& a_group="");

		const string& getChildGroup(UIComponent* a_child) const;
		const vector<UIComponent*>& getGroup(const string& a_group) const;

		bool removeChild(UIComponent* a_child);
		bool removeChild(int a_index);

		shared_ptr<UIComponent> getChild(int i, const string& a_group="") const;
		shared_ptr<UIComponent> getChild(const UIComponent* a_comp) const;
		int getChildIndex(const UIComponent* a_comp) const;

		int numChildren() const;

		const vector<shared_ptr<UIComponent>>& children() const;

		UIComponent* parent() const;

		UILayout* layout() const;

		void setLayout(UILayout* a_layout);

		void setLayout(shared_ptr<UILayout> a_layout);
		virtual void performLayout(NVGcontext* a_nvg);

		shared_ptr<UIComponent> getSharedPtr();

		virtual bool contains(const UICoord& a_pt) const;

		UIComponent* findChild(const UICoord& a_pt, const string& a_filterGroup="") const;
		UIComponent* findChildRecursive(const UICoord& a_pt) const;

		Vector2i getRelPos() const;

		Vector2i getRelCenter() const;

		void setRelPos(const Vector2i& a_pos);

		Vector2i getAbsPos() const;

		Vector2i getAbsCenter() const;

		void move(const Vector2i& a_dist);

		Vector2i size() const;

		Vector2i minSize() const;

		Vector2i maxSize() const;

		/**
		 * Attempts to set the component's size, obeying the component's min and max size constraints.
		 * To leave a dimensions the same, set it to -1.
		 */
		void setSize(const Vector2i& a_size);

		void grow(const Vector2i& a_amt);

		NVGcontext* nvgContext() const;
		sf::RenderWindow* sfmlWindow() const;
		shared_ptr<Theme> theme() const;

		bool visible() const;
		bool focused() const;

		bool hovered() const;

		void setVisible(bool a_visible);

		virtual bool onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor);
		virtual bool onMouseMove(const UICoord& a_relCursor, const Vector2i& a_diffCursor);
		virtual void onMouseEnter(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isEntering);
		virtual UIComponent* onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick);
		virtual bool onMouseUp(const UICoord& a_relCursor, const Vector2i& a_diffCursor);
		virtual bool onMouseScroll(const UICoord& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt);
		virtual bool onTextEntered(sf::Uint32 a_unicode);
		virtual bool onKeyDown(const sf::Event::KeyEvent& a_key);
		virtual bool onKeyUp(const sf::Event::KeyEvent& a_key);

		virtual void notifyChildResized(UIComponent* a_child) {};

		virtual void onFocusEvent(bool a_isFocused);

		void bringToFront(UIComponent* a_child);
		void pushToBack(UIComponent* a_child);

		int getZOrder(UIComponent* a_child);

		/**
		 * Sets a components z-order. 
		 * If the component already has the given z-order, it is brought to the front of that z-order.
		 * \param toFront When true, pushes the component to the front of the z-order, and pushes to the back otherwise.
		 */
		void setZOrder(UIComponent* a_child, int a_zorder, bool toFront = true);

		/**
		* Sets maximum size constraint. To unconstrain a dimension, set it to a negative number.
		*/
		void setMaxSize(const Vector2i& a_maxSize);

		/**
		* Sets minimum size constraint. To unconstrain a dimension, set it to a negative number.
		*/
		void setMinSize(const Vector2i& a_minSize);

	protected:
		virtual void draw(NVGcontext* a_nvg) {};
		virtual void setChildrenStyles(NVGcontext* a_nvg) {};

	private:
		virtual void _onChildMoved(UIComponent* a_child) {};

		virtual void _onAddChild(shared_ptr<UIComponent> a_newchild) {};

		virtual void _onRemoveChild() {};

		virtual void _onResize() {};

	protected:
		UIComponent* m_parent;
		MainWindow* m_window;
		shared_ptr<UILayout> m_layout;
		vector<shared_ptr<UIComponent> > m_children;
		map<int, list<UIComponent*> > m_ZPlaneMap;
		map<UIComponent*, int> m_reverseZPlaneMap;
		map<string, vector<UIComponent*> > m_groupMap;
		map<UIComponent*, string> m_reverseGroupMap;

		bool m_visible, m_focused, m_hovered;
		Vector2i m_pos, m_size;
		Vector2i m_minSize, m_maxSize;
	};

	class UIResizeHandle : public UIComponent
	{
	public:
		UIResizeHandle(MainWindow* a_window)
			: UIComponent{a_window} {
			setMinSize({10,10});
		}

		bool onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override {
			m_dragCallback(a_relCursor, a_diffCursor);
			return true;
		}

		UIComponent* onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override {
			return this;
		}

		void setDragCallback(const function<void(const UICoord&, const Vector2i&)>& a_callback) {
			m_dragCallback = a_callback;
		}

	protected:
		void draw(NVGcontext* a_nvg) override {
			nvgBeginPath(a_nvg);
			if (hovered())
				nvgStrokeColor(a_nvg, Color(1.0f, 1.0f));
			else
				nvgStrokeColor(a_nvg, Color(1.0, 0.7f));
			nvgMoveTo(a_nvg, 0.0f, size()[1]);
			nvgLineTo(a_nvg, size()[0], 0.0f);
			nvgMoveTo(a_nvg, size()[0] / 3.0f, size()[1]);
			nvgLineTo(a_nvg, size()[0], size()[1] / 3.0f);
			nvgMoveTo(a_nvg, size()[0] * 2.0f / 3.0f, size()[1]);
			nvgLineTo(a_nvg, size()[0], size()[1] * 2.0f / 3.0f);
			nvgStroke(a_nvg);
		}

	private:
		function<void(const UICoord& a_relCursor, const Vector2i& a_diffCursor)> m_dragCallback;
	};
};
#endif
