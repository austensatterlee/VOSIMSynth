#include "UICell.h"

synui::UICell::UICell(MainWindow* a_window) : 
	UIComponent(a_window),
	m_greedyChild(nullptr),
	m_childResizePolicy(CNONE),
	m_selfMinSizePolicy(SFIT), 
	m_selfResizePolicy(SRNONE),
	m_padding{0,0,0,0},
	m_childSpacing(1),
	m_maxChildSize(0,0),
	m_maxChildMinSize(0,0),
	m_packOnChildResize(true)
{}

void synui::UICell::setGreedyChild(UIComponent* a_child)
{
	if (m_greedyChild != a_child) {
		m_greedyChild = a_child;
		pack();
	}
}

void synui::UICell::setChildResizePolicy(ChildResizePolicy a_policy)
{
	if (m_childResizePolicy != a_policy) {
		m_childResizePolicy = a_policy;
		pack();
	}
}

void synui::UICell::setSelfMinSizePolicy(SelfMinSizePolicy a_policy)
{
	if (m_selfMinSizePolicy != a_policy) {
		m_selfMinSizePolicy = a_policy;
		pack();
	}
}

void synui::UICell::setSelfResizePolicy(SelfResizePolicy a_policy)
{
	if (m_selfResizePolicy != a_policy) {
		m_selfResizePolicy = a_policy;
		pack();
	}
}

void synui::UICell::setPadding(const Vector4i& a_padding)
{
	if (m_padding != a_padding) {
		m_padding = a_padding;
		pack();
	}
}

void synui::UICell::setChildrenSpacing(int a_padAmt)
{
	if (m_childSpacing != a_padAmt) {
		m_childSpacing = a_padAmt;
		pack();
	}
}

Eigen::Vector2i synui::UICell::innerSize() const
{
	return{ size()[0] - padding()[0] - padding()[2], size()[1] - padding()[1] - padding()[3] };			
}

const Eigen::Vector4i& synui::UICell::padding() const
{
	return m_padding;
}

void synui::UICol::layoutChildren_()
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
	int extraHeight = innerSize().y() - y;
	if (m_greedyChild) {
		int greedyCellHeight = m_greedyChild->size()[1] + extraHeight;

		m_greedyChild->setSize({ -1, greedyCellHeight });
	}
}

void synui::UICol::resizeChildren_()
{
	bool oldPackOnChildResize = m_packOnChildResize;
	m_packOnChildResize = false;
	// Resize children according to ChildResizePolicy
	for (shared_ptr<UIComponent> child : m_children) {
		switch (m_childResizePolicy) {
		case CMIN:
			child->setSize({child->minSize()[0], -1});
			break;
		case CMATCHMIN:
			child->setSize({ m_maxChildMinSize[0], -1 });
			break;
		case CMATCHMAX:
			child->setSize({ syn::MIN(innerSize()[0],m_maxChildSize[0]), -1 });
			break;
		case CMAX:
			child->setSize({ innerSize()[0], -1 });
			break;
		default: break;
		}
	}
	m_packOnChildResize = oldPackOnChildResize;
}

void synui::UICol::updateMinSize_()
{
	m_maxChildMinSize = { 0,0 };
	m_maxChildSize = { 0,0 };
	Vector2i m_extent = { 0,0 };
	int y = m_padding[1];
	for (shared_ptr<UIComponent> child : m_children) {
		if (child->visible()) {
			m_maxChildSize = m_maxChildSize.cwiseMax(child->size());
			m_maxChildMinSize = m_maxChildMinSize.cwiseMax(child->minSize());
			m_extent = m_extent.cwiseMax(child->getRelPos() + child->size());
			y += child->size()[1] + m_childSpacing;
		}
	}
	m_extent += Vector2i{m_padding[2], m_padding[3]};
	Vector2i pad_wh = Vector2i{ m_padding[0] + m_padding[2],m_padding[1] + m_padding[3] };
	int minWidth = m_maxChildMinSize[0];
	if (m_selfMinSizePolicy != SNONE)
		setMinSize(Vector2i{ minWidth, y - m_childSpacing } + pad_wh);

	// Shrink cell according to resize policy
	if (m_selfResizePolicy == SRMIN) {
		setSize(minSize());
	}else if (m_selfResizePolicy == SRFIT) {
		setSize(m_extent);
	}	
}

void synui::UIRow::layoutChildren_()
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
	int extraWidth = innerSize().x() - x;
	if (m_greedyChild) {
		int greedyCellWidth = m_greedyChild->size()[0] + extraWidth;

		m_greedyChild->setSize({ greedyCellWidth, -1 });
	}
}

void synui::UIRow::resizeChildren_()
{
	bool oldPackOnChildResize = m_packOnChildResize;
	m_packOnChildResize = false;
	// Resize children according to ChildResizePolicy
	for (shared_ptr<UIComponent> child : m_children) {
		switch (m_childResizePolicy) {
		case CMIN:
			child->setSize({ -1, child->minSize()[1] });
			break;
		case CMATCHMIN:
			child->setSize({ -1, m_maxChildMinSize[1] });
			break;
		case CMATCHMAX:
			child->setSize({ -1, syn::MIN(innerSize()[1],m_maxChildSize[1]) });
			break;
		case CMAX:
			child->setSize({ -1, innerSize()[1] });
			break;
		default: break;
		}
	}
	m_packOnChildResize = oldPackOnChildResize;
}

void synui::UIRow::updateMinSize_()
{
	m_maxChildMinSize = { 0,0 };
	m_maxChildSize = { 0,0 };
	Vector2i m_extent = {0,0};
	int x = m_padding[0];
	for (shared_ptr<UIComponent> child : m_children) {
		if (child->visible()) {
			x += child->size()[0] + m_childSpacing;
			m_extent = m_extent.cwiseMax(child->getRelPos() + child->size());
			m_maxChildSize = m_maxChildSize.cwiseMax(child->size());
			m_maxChildMinSize = m_maxChildMinSize.cwiseMax(child->minSize());
		}
	}
	m_extent += Vector2i{ m_padding[2], m_padding[3] };

	// Update cell minimum size
	Vector2i pad_wh = Vector2i{ m_padding[0] + m_padding[2],m_padding[1] + m_padding[3] };
	int minHeight = m_maxChildMinSize[1];
	if (m_selfMinSizePolicy != SNONE)
		setMinSize(Vector2i{ x - m_childSpacing, minHeight } + pad_wh);
	
	// Shrink cell according to resize policy
	if (m_selfResizePolicy == SRMIN) {
		setSize(minSize());
	}else if(m_selfResizePolicy == SRFIT){
		setSize(m_extent);
	}
}
