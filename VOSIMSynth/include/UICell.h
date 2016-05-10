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
 *  \file UILayout.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 05/2016
 */

#ifndef __UILAYOUT__
#define __UILAYOUT__
#include "UIComponent.h"

namespace syn
{
	class UICell : public UIComponent
	{
	public:
		enum ChildResizePolicy
		{
			CMIN,
			CNONE,
			CMATCHMIN,
			CMATCHMAX,
			CMAX
		};

		enum SelfResizePolicy
		{
			SMIN,
			SNONE,			
			SFIT
		};

	public:
		UICell(VOSIMWindow* a_window)
			: UIComponent(a_window), m_greedyChild(nullptr), m_childResizePolicy(CNONE), m_selfResizePolicy(SFIT), m_greedyChildAlign(NVG_ALIGN_TOP), m_padding{ 0,0,0,0 }
		{}

		void notifyChildResized(UIComponent* a_child) override {
			pack();
		}

		virtual void pack() = 0;

		void setGreedyChild(UIComponent* a_child, int a_align = NVG_ALIGN_TOP) {
			if (m_greedyChild != a_child || m_greedyChildAlign != a_align) {
				m_greedyChild = a_child;
				m_greedyChildAlign = a_align;
				pack();
			}
		}

		void setChildResizePolicy(ChildResizePolicy a_policy) {
			if (m_childResizePolicy != a_policy) {
				m_childResizePolicy = a_policy;
				pack();
			}
		}

		void setSelfResizePolicy(SelfResizePolicy a_policy) {
			if (m_selfResizePolicy != a_policy) {
				m_selfResizePolicy = a_policy;
				pack();
			}
		}

		void setPadding(const Vector4i& a_padding) {
			if (m_padding != a_padding) {
				m_padding = a_padding;
				pack();
			}
		}

		const Vector4i& padding() const { return m_padding; }

	protected:
		UIComponent* m_greedyChild;
		ChildResizePolicy m_childResizePolicy;
		SelfResizePolicy m_selfResizePolicy;
		int m_greedyChildAlign;
		Vector4i m_padding;
	};

	class UICol : public UICell
	{
	public:
		UICol(VOSIMWindow* a_window)
			: UICell(a_window), m_maxChildWidth(0), m_minChildWidth(0) {}

		void pack() override {
			int y = m_padding[1];
			m_maxChildWidth = 0;
			m_minChildWidth = 0;
			// Reposition units
			for (shared_ptr<UIComponent> child : m_children) {
				child->setRelPos({0,y});
				y += child->size()[1];
				m_maxChildWidth = MAX(m_maxChildWidth, child->size()[0]);
				m_minChildWidth = MAX(m_minChildWidth, child->minSize()[0]);				
			}

			// Update cell minimum size
			int width;
			if (m_selfResizePolicy == SMIN) 
				width = m_minChildWidth;
			else
				width = m_maxChildWidth;
			setMinSize_({width + m_padding[0] + m_padding[2],y + m_padding[1] + m_padding[3]});

			Vector2i freeSpace = size() - minSize();
			// Shrink cell according to resize policy
			if (m_selfResizePolicy != SNONE && freeSpace.any()) {
				setSize(minSize());
			}

			// If there's extra space left over, expand the greedy child
			// If there is no greedy child, shrink the cell to get rid of the extra space
			freeSpace = size() - minSize();
			int extraHeight = freeSpace[1];
			if (m_selfResizePolicy == SNONE && m_greedyChild && extraHeight>0) {
				int greedyCellHeight = m_greedyChild->size()[1] + extraHeight;

				// Align child if it doesn't take up the whole cell
				int greedyCellShift;
				switch (m_greedyChildAlign) {
				case NVG_ALIGN_BOTTOM:
					greedyCellShift = greedyCellHeight - m_greedyChild->size()[1];
					break;
				case NVG_ALIGN_MIDDLE:
					greedyCellShift = (greedyCellHeight - m_greedyChild->size()[1]) / 2;
					break;
				default:
					greedyCellShift = 0;
				}
				m_greedyChild->move({0, greedyCellShift});

				// Shift children after the greedy one
				int i = getChildIndex(m_greedyChild) + 1;
				for (; i < m_children.size(); i++) {
					shared_ptr<UIComponent> child = m_children[i];
					child->move({0, extraHeight});
				}
			} 
		}

	private:
		void _onResize() override {
			for (shared_ptr<UIComponent> child : m_children) {
				switch(m_childResizePolicy) {
				case CMIN:
					child->setSize(child->minSize());
					break;
				case CMATCHMIN:
					child->setSize({ m_minChildWidth, child->size()[1] });
					break;
				case CMATCHMAX:
					child->setSize({ m_maxChildWidth, child->size()[1] });
					break;
				case CMAX:
					child->setSize({ size()[0] - m_padding[0] - m_padding[2], child->size()[1] });
					break;
				default: break;
				}
			}			
		}
	private:
		int m_maxChildWidth;
		int m_minChildWidth;
	};

	class UIRow : public UICell
	{
	public:
		UIRow(VOSIMWindow* a_window)
			: UICell(a_window), m_maxChildHeight(0), m_minChildHeight(0) {}

		void pack() override {
			int x = m_padding[0];
			m_maxChildHeight = 0;
			m_minChildHeight = 0;
			for (shared_ptr<UIComponent> child : m_children) {
				child->setRelPos({x,0});
				x += child->size()[0];
				m_maxChildHeight = MAX(m_maxChildHeight, child->size()[1]);
				m_minChildHeight = MAX(m_minChildHeight, child->minSize()[1]);
			}

			// Update cell minimum size
			int height;
			if (m_selfResizePolicy == SMIN)
				height = m_minChildHeight;
			else
				height = m_maxChildHeight;
			setMinSize_({ x + m_padding[0] + m_padding[2],height + m_padding[1] + m_padding[3] });
			
			// Shrink cell according to resize policy
			Vector2i freeSpace = size() - minSize();
			if (m_selfResizePolicy != SNONE && freeSpace.any()) {
				setSize(minSize());
			}

			// If there's extra space left over, expand the greedy child
			// If there is no greedy child, shrink the cell to get rid of the extra space
			freeSpace = size() - minSize();
			int extraWidth = freeSpace[0];
			if (m_selfResizePolicy == SNONE && m_greedyChild && extraWidth>0) {
				int greedyCellWidth = m_greedyChild->size()[0] + extraWidth;

				// Align child if it doesn't take up the whole cell
				int greedyCellShift;
				switch (m_greedyChildAlign) {
				case NVG_ALIGN_BOTTOM:
					greedyCellShift = greedyCellWidth - m_greedyChild->size()[0];
					break;
				case NVG_ALIGN_MIDDLE:
					greedyCellShift = (greedyCellWidth - m_greedyChild->size()[0]) / 2;
					break;
				default:
					greedyCellShift = 0;
				}
				m_greedyChild->move({greedyCellShift,0});

				// Shift children after the greedy one
				int i = getChildIndex(m_greedyChild) + 1;
				for (; i < m_children.size(); i++) {
					shared_ptr<UIComponent> child = m_children[i];
					child->move({extraWidth, 0});
				}
			}
		}
	private:
		void _onResize() override {
			for (shared_ptr<UIComponent> child : m_children) {
				switch (m_childResizePolicy) {
				case CMIN:
					child->setSize(child->minSize());
					break;
				case CMATCHMIN:
					child->setSize({ child->size()[0], m_minChildHeight });
					break;
				case CMATCHMAX:
					child->setSize({ child->size()[0], m_maxChildHeight });
					break;
				case CMAX:
					child->setSize({ child->size()[0], size()[1] - m_padding[1] - m_padding[3] });
					break;
				default: break;
				}
				
			}
		}
	private:
		int m_maxChildHeight;
		int m_minChildHeight;
	};
}


#endif
