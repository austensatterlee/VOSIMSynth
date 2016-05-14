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
 *  \file UICell.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 05/2016
 */

#ifndef __UICELL__
#define __UICELL__
#include "UIComponent.h"

namespace syn
{
	/**
	 * Basic layout management unit. Automatically position children along one dimension. 
	 * The sizing of itself and its children is automatically managed according to the specified ChildResizePolicy and SelfResizePolicy.
	 * UIRow and UICol objects can be embedded into one another to create flexible grid layouts.
	 */
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
			: UIComponent(a_window), m_greedyChild(nullptr), m_childResizePolicy(CNONE), m_selfResizePolicy(SFIT), m_greedyChildAlign(NVG_ALIGN_TOP), m_padding{0,0,0,0}, m_childSpacing(1) {}

		void notifyChildResized(UIComponent* a_child) override {
			pack();
		}

		/**
		 * Recomputes the layout. Note that in most cases packing is done automatically when the cell is changed.
		 */
		virtual void pack() = 0;

		/**
		 * Specifies one of the children to be the greedy child.
		 *
		 * The division of the greedy child will expand to take up any extra space.
		 * While the size of the child itself is not changed, the cell spaces the children as if it were.
		 * The alignment parameter specifies where to place the child within the extra space.
		 *
		 * \param a_align For cols: One of NVG_ALIGN_TOP, NVG_ALIGN_BOTTOM, NVG_ALIGN_MIDDLE. For rows: One of NVG_ALIGN_LEFT, NVG_ALIGN_CENTER, NVG_ALIGN_RIGHT.
		 */
		void setGreedyChild(UIComponent* a_child, int a_align) {
			if (m_greedyChild != a_child || m_greedyChildAlign != a_align) {
				m_greedyChild = a_child;
				m_greedyChildAlign = a_align;
				pack();
			}
		}

		/**
		 * The child resize policy specifies how the children are resized relative to each other.
		 * 		 
		 * CMIN			Children are set to their minimum sizes.
		 * CNONE		Children are never resized.
		 * CMATCHMIN	Children are made as small as possible under the constraint that they all have the same size.
		 * CMATCHMAX	Children are made as large as possible under the constraint that they all have the same size.
		 * CMAX			Children are made as large as possible.
		 *
		 * Note that only one dimension of the children is touched. UIRow's will change its children's heights, UICol's will change their widths.
		 * Also note that the behavior of CMATCHMAX and CMATCH is constrained by the self resizing policy in that children will not be made larger
		 * than the size of the cell. (\see setSelfResizePolicy).
		 *
		 */
		void setChildResizePolicy(ChildResizePolicy a_policy) {
			if (m_childResizePolicy != a_policy) {
				m_childResizePolicy = a_policy;
				pack();
			}
		}

		/**
		* The self resize policy specifies how the cell resizes itself and how it computes its minimum size.
		*
		* SMIN			Minimum size is computed using the minimum sizes of the children. The cell resizes itself to its minimum size.
		* SNONE			Minimum size is computed using the current sizes of the children. The cell does not resize itself.
		* SFIT			Minimum size is computed using the current sizes of the children. The cell resizes itself to its minimum size.
		*
		*/
		void setSelfResizePolicy(SelfResizePolicy a_policy) {
			if (m_selfResizePolicy != a_policy) {
				m_selfResizePolicy = a_policy;
				pack();
			}
		}

		/**
		 * Specifies how much space to leave around the cell's edges.
		 *
		 * \param a_padding A 4-dimensional vector in {left, top, right, bottom} order.
		 */
		void setPadding(const Vector4i& a_padding) {
			if (m_padding != a_padding) {
				m_padding = a_padding;
				pack();
			}
		}

		/**
		 * Specifies how much space to leave in between children.
		 */
		void setChildrenSpacing(int a_padAmt) {
			if (m_childSpacing != a_padAmt) {
				m_childSpacing = a_padAmt;
				pack();
			}
		}

		const Vector4i& padding() const {
			return m_padding;
		}

	protected:
		UIComponent* m_greedyChild;
		ChildResizePolicy m_childResizePolicy;
		SelfResizePolicy m_selfResizePolicy;
		int m_greedyChildAlign;
		Vector4i m_padding;
		int m_childSpacing;
	};

	/**
	 * Positions children vertically.
	 */
	class UICol : public UICell
	{
	public:
		UICol(VOSIMWindow* a_window)
			: UICell(a_window), m_maxChildWidth(0), m_minChildWidth(0) {}

		void pack() override {
			// Reposition units and compute min and max child dimensions
			m_maxChildWidth = 0;
			m_minChildWidth = 0;
			int y = m_padding[1];
			for (shared_ptr<UIComponent> child : m_children) {
				child->setRelPos({m_padding[0],y});
				y += child->size()[1] + m_childSpacing;
				m_maxChildWidth = MAX(m_maxChildWidth, child->size()[0]);
				m_minChildWidth = MAX(m_minChildWidth, child->minSize()[0]);
			}

			// Update cell minimum size
			Vector2i pad_wh = Vector2i{m_padding[0] + m_padding[2],m_padding[1] + m_padding[3]};
			int width;
			if (m_selfResizePolicy == SMIN)
				width = m_minChildWidth;
			else
				width = m_maxChildWidth;
			setMinSize_(Vector2i{width,y - m_childSpacing} + pad_wh);

			Vector2i freeSpace = size() - minSize();
			// Shrink cell according to resize policy
			if (m_selfResizePolicy != SNONE && freeSpace.any()) {
				setSize(minSize());
			}

			// If there's extra space left over, expand the greedy child
			// If there is no greedy child, shrink the cell to get rid of the extra space
			freeSpace = size() - pad_wh - minSize();
			int extraHeight = freeSpace[1];
			if (m_selfResizePolicy == SNONE && m_greedyChild && extraHeight > 0) {
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
				case NVG_ALIGN_LEFT:
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
				switch (m_childResizePolicy) {
				case CMIN:
					child->setSize(child->minSize());
					break;
				case CMATCHMIN:
					child->setSize({m_minChildWidth, -1});
					break;
				case CMATCHMAX:
					child->setSize({MIN(size()[0] - m_padding[0] - m_padding[2],m_maxChildWidth), -1});
					break;
				case CMAX:
					child->setSize({size()[0] - m_padding[0] - m_padding[2], -1});
					break;
				default: break;
				}
			}
			pack();
		}

	private:
		int m_maxChildWidth;
		int m_minChildWidth;
	};

	/**
	 * Positions children horizontally.
	 */
	class UIRow : public UICell
	{
	public:
		UIRow(VOSIMWindow* a_window)
			: UICell(a_window), m_maxChildHeight(0), m_minChildHeight(0) {}

		void pack() override {
			// Reposition units and compute min and max child dimensions
			m_maxChildHeight = 0;
			m_minChildHeight = 0;
			int x = m_padding[0];
			for (shared_ptr<UIComponent> child : m_children) {
				child->setRelPos({x,m_padding[1]});
				x += child->size()[0] + m_childSpacing;
				m_maxChildHeight = MAX(m_maxChildHeight, child->size()[1]);
				m_minChildHeight = MAX(m_minChildHeight, child->minSize()[1]);
			}

			// Update cell minimum size
			Vector2i pad_wh = Vector2i{m_padding[0] + m_padding[2],m_padding[1] + m_padding[3]};
			int height;
			if (m_selfResizePolicy == SMIN)
				height = m_minChildHeight;
			else
				height = m_maxChildHeight;
			setMinSize_(Vector2i{x - m_childSpacing,height} + pad_wh);

			// Shrink cell according to resize policy
			Vector2i freeSpace = size() - pad_wh - minSize();
			if (m_selfResizePolicy != SNONE && freeSpace.any()) {
				setSize(minSize());
			}

			// If there's extra space left over, expand the greedy child
			// If there is no greedy child, shrink the cell to get rid of the extra space
			freeSpace = size() - minSize();
			int extraWidth = freeSpace[0];
			if (m_selfResizePolicy == SNONE && m_greedyChild && extraWidth > 0) {
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
					child->setSize({child->size()[0], m_minChildHeight});
					break;
				case CMATCHMAX:
					child->setSize({child->size()[0], MIN(size()[1] - m_padding[1] - m_padding[3],m_maxChildHeight)});
					break;
				case CMAX:
					child->setSize({child->size()[0], size()[1] - m_padding[1] - m_padding[3]});
					break;
				default: break;
				}

			}
			pack();
		}

	private:
		int m_maxChildHeight;
		int m_minChildHeight;
	};
}


#endif
