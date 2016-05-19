#include "UICell.h"

syn::UICell::UICell(VOSIMWindow* a_window): 
	UIComponent(a_window),
	m_greedyChild(nullptr),
	m_childResizePolicy(CMATCHMAX),
	m_selfMinSizePolicy(SFIT), 
	m_selfResizePolicy(SPASSIVE),
	m_greedyChildAlign(NVG_ALIGN_TOP),
	m_padding{0,0,0,0},
	m_childSpacing(1),
	m_maxChildMinSize(0,0),
	m_maxChildSize(0,0)
{}

void syn::UICell::setGreedyChild(UIComponent* a_child, int a_align)
{
	if (m_greedyChild != a_child || m_greedyChildAlign != a_align) {
		m_greedyChild = a_child;
		m_greedyChildAlign = a_align;
		pack();
	}
}

void syn::UICell::setChildResizePolicy(ChildResizePolicy a_policy)
{
	if (m_childResizePolicy != a_policy) {
		m_childResizePolicy = a_policy;
		pack();
	}
}

void syn::UICell::setSelfMinSizePolicy(SelfMinSizePolicy a_policy)
{
	if (m_selfMinSizePolicy != a_policy) {
		m_selfMinSizePolicy = a_policy;
		pack();
	}
}

void syn::UICell::setSelfResizePolicy(SelfResizePolicy a_policy)
{
	if (m_selfResizePolicy != a_policy) {
		m_selfResizePolicy = a_policy;
		pack();
	}
}

void syn::UICell::setPadding(const Vector4i& a_padding)
{
	if (m_padding != a_padding) {
		m_padding = a_padding;
		pack();
	}
}

void syn::UICell::setChildrenSpacing(int a_padAmt)
{
	if (m_childSpacing != a_padAmt) {
		m_childSpacing = a_padAmt;
		pack();
	}
}

Eigen::Vector2i syn::UICell::innerSize() const
{
	return{ size()[0] - padding()[0] - padding()[2], size()[1] - padding()[1] - padding()[3] };			
}

const Eigen::Vector4i& syn::UICell::padding() const
{
	return m_padding;
}

void syn::UICol::layoutChildren_()
{
	int y = m_padding[1];
	for (shared_ptr<UIComponent> child : m_children) {
		if (child->visible()) {
			child->setRelPos({ m_padding[0],y });
			y += child->size()[1] + m_childSpacing;
		}
	}

	// If there's extra space left over, expand the greedy child
	// If there is no greedy child, shrink the cell to get rid of the extra space
	Vector2i freeSpace = innerSize() - minSize();
	int extraHeight = freeSpace[1];
	if (m_greedyChild && extraHeight > 0) {
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
		m_greedyChild->move({ 0, greedyCellShift });

		// Shift children after the greedy one
		int i = getChildIndex(m_greedyChild) + 1;
		for (; i < m_children.size(); i++) {
			shared_ptr<UIComponent> child = m_children[i];
			child->move({ 0, extraHeight });
		}
	}
}

void syn::UICol::resizeChildren_()
{
	// Resize children according to ChildResizePolicy
	for (shared_ptr<UIComponent> child : m_children) {
		switch (m_childResizePolicy) {
		case CMIN:
			child->setSize(child->minSize());
			break;
		case CMATCHMIN:
			child->setSize({ m_maxChildMinSize[0], -1 });
			break;
		case CMATCHMAX:
			child->setSize({ MIN(innerSize()[0],m_maxChildSize[0]), -1 });
			break;
		case CMAX:
			child->setSize({ innerSize()[0], -1 });
			break;
		default: break;
		}
	}
}

void syn::UICol::updateMinSize_()
{
	m_maxChildMinSize = { 0,0 };
	m_maxChildSize = { 0,0 };
	int y = m_padding[1];
	for (shared_ptr<UIComponent> child : m_children) {
		if (child->visible()) {
			m_maxChildSize = m_maxChildSize.cwiseMax(child->size());
			m_maxChildMinSize = m_maxChildMinSize.cwiseMax(child->minSize());
			y += child->size()[1] + m_childSpacing;
		}
	}
	Vector2i pad_wh = Vector2i{ m_padding[0] + m_padding[2],m_padding[1] + m_padding[3] };
	int width;
	if (m_selfMinSizePolicy == SMIN)
		width = m_maxChildMinSize[0];
	else
		width = m_maxChildSize[0];
	if (m_selfMinSizePolicy != SNONE)
		setMinSize(Vector2i{ width, y - m_childSpacing } + pad_wh);

	Vector2i freeSpace = innerSize() - minSize();
	// Shrink cell according to resize policy
	if (m_selfResizePolicy == SACTIVE && freeSpace.any()) {
		setSize(minSize());
	}
}

void syn::UIRow::layoutChildren_()
{
	int x = m_padding[0];
	for (shared_ptr<UIComponent> child : m_children) {
		if (child->visible()) {
			child->setRelPos({ x,m_padding[1] });
			x += child->size()[0] + m_childSpacing;
		}
	}

	// If there's extra space left over, expand the greedy child
	// If there is no greedy child, shrink the cell to get rid of the extra space
	Vector2i freeSpace = innerSize() - minSize();
	int extraWidth = freeSpace[0];
	if (m_greedyChild && extraWidth > 0) {
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
		m_greedyChild->move({ greedyCellShift,0 });

		// Shift children after the greedy one
		int i = getChildIndex(m_greedyChild) + 1;
		for (; i < m_children.size(); i++) {
			shared_ptr<UIComponent> child = m_children[i];
			child->move({ extraWidth, 0 });
		}
	}
}

void syn::UIRow::resizeChildren_()
{
	// Resize children according to ChildResizePolicy
	for (shared_ptr<UIComponent> child : m_children) {
		switch (m_childResizePolicy) {
		case CMIN:
			child->setSize(child->minSize());
			break;
		case CMATCHMIN:
			child->setSize({ -1, m_maxChildMinSize[1] });
			break;
		case CMATCHMAX:
			child->setSize({ -1, MIN(innerSize()[1],m_maxChildSize[1]) });
			break;
		case CMAX:
			child->setSize({ -1, innerSize()[1] });
			break;
		default: break;
		}
	}
}

void syn::UIRow::updateMinSize_()
{
	m_maxChildMinSize = { 0,0 };
	m_maxChildSize = { 0,0 };
	int x = m_padding[0];
	for (shared_ptr<UIComponent> child : m_children) {
		if (child->visible()) {
			x += child->size()[0] + m_childSpacing;
			m_maxChildSize = m_maxChildSize.cwiseMax(child->size());
			m_maxChildMinSize = m_maxChildMinSize.cwiseMax(child->minSize());
		}
	}

	// Update cell minimum size
	Vector2i pad_wh = Vector2i{ m_padding[0] + m_padding[2],m_padding[1] + m_padding[3] };
	int height;
	if (m_selfMinSizePolicy == SMIN)
		height = m_maxChildMinSize[1];
	else
		height = m_maxChildSize[1];
	if (m_selfMinSizePolicy != SNONE)
		setMinSize(Vector2i{ x - m_childSpacing,height } +pad_wh);
	
	// Shrink cell according to resize policy
	Vector2i freeSpace = innerSize() - minSize();
	if (m_selfResizePolicy == SACTIVE && freeSpace.any()) {
		setSize(minSize());
	}
}
