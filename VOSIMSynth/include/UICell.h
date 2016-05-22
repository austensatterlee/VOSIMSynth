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
	 * The sizing of itself and its children is automatically managed according to the specified ChildResizePolicy and SelfMinSizePolicy.
	 * UIRow and UICol objects can be embedded into one another to create flexible grid layouts.
	 */
	class UICell : public UIComponent
	{		
	public:
		enum ChildResizePolicy
		{
			CNONE,
			CMIN,
			CMATCHMIN,
			CMATCHMAX,
			CMAX
		};

		enum SelfMinSizePolicy
		{
			SNONE,
			SMIN,
			SFIT
		};

		enum SelfResizePolicy
		{
			SRNONE,
			SRMIN,
			SRFIT
		};

	public:
		UICell(VOSIMWindow* a_window);

		void notifyChildResized(UIComponent* a_child) override {
			if (m_packOnChildResize) {
				pack();
			}
		}

		/**
		 * Recomputes the layout. Note that in most cases packing is done automatically when the cell is changed.
		 */
		void pack()
		{
			updateMinSize_();
			layoutChildren_();
			resizeChildren_();
		}

		/**
		 * Specifies one of the children to be the greedy child.
		 *
		 * The division of the greedy child will expand to take up any extra space.
		 * While the size of the child itself is not changed, the cell spaces the children as if it were.
		 * The alignment parameter specifies where to place the child within the extra space.
		 *
		 * Note that this will not have any effect if the SelfMinSizePolicy is anything other than SNONE, since the other policies remove extra space by shrinking the cell.
		 *
		 * \param a_align For cols: One of NVG_ALIGN_TOP, NVG_ALIGN_BOTTOM, NVG_ALIGN_MIDDLE. For rows: One of NVG_ALIGN_LEFT, NVG_ALIGN_CENTER, NVG_ALIGN_RIGHT.
		 */
		void setGreedyChild(UIComponent* a_child, int a_align);

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
		 * than the size of the cell. (\see setSelfMinSizePolicy).
		 *
		 */
		void setChildResizePolicy(ChildResizePolicy a_policy);

		/**
		 * The self "minsize" policy specifies how the cell calculates its minimum size.
		 *
		 * SMIN			Minimum size is computed using the minimum sizes of the children.
		 * SNONE			Minimum size is not computed (it can be set externally instead).
		 * SFIT			Minimum size is computed using the current sizes of the children.
		 *
		 */
		void setSelfMinSizePolicy(SelfMinSizePolicy a_policy);

		/**
		 * The self resize policy specifies whether or not the cell manages its own size.
		 */
		void setSelfResizePolicy(SelfResizePolicy a_policy);

		/**
		 * Specifies how much space to leave around the cell's edges.
		 *
		 * \param a_padding A 4-dimensional vector in {left, top, right, bottom} order.
		 */
		void setPadding(const Vector4i& a_padding);

		/**
		 * Specifies how much space to leave in between children.
		 */
		void setChildrenSpacing(int a_padAmt);

		Vector2i innerSize() const;

		const Vector4i& padding() const;
	protected:
		virtual void layoutChildren_() = 0;
		virtual void resizeChildren_() = 0;
		virtual void updateMinSize_() = 0;
	private:
		void _onAddChild(shared_ptr<UIComponent> a_newchild) override {
			pack();
		}

		void _onRemoveChild() override {
			pack();
		}

		void _onResize() override {
			layoutChildren_();
			resizeChildren_();
			updateMinSize_();
		}

	protected:
		UIComponent* m_greedyChild;
		ChildResizePolicy m_childResizePolicy;
		SelfMinSizePolicy m_selfMinSizePolicy;
		SelfResizePolicy m_selfResizePolicy;
		int m_greedyChildAlign;
		Vector4i m_padding;
		int m_childSpacing;
		Vector2i m_maxChildSize;
		Vector2i m_maxChildMinSize;

		bool m_packOnChildResize;
	};

	/**
	 * Positions children vertically.
	 */
	class UICol : public UICell
	{
	public:
		UICol(VOSIMWindow* a_window)
			: UICell(a_window)
		{}
	protected:
		void layoutChildren_() override;
		void resizeChildren_() override;
		void updateMinSize_() override;
	};

	/**
	 * Positions children horizontally.
	 */
	class UIRow : public UICell
	{
	public:
		UIRow(VOSIMWindow* a_window)
			: UICell(a_window)
		{}
	protected:
		void layoutChildren_() override;
		void resizeChildren_() override;
		void updateMinSize_() override;
	};
}


#endif
