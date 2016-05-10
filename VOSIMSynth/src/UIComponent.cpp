#include "UIComponent.h"
//#define DRAW_COMPONENT_BOUNDS

namespace syn
{
	UIComponent::UIComponent(VOSIMWindow* a_window):
		m_parent(nullptr), m_window(a_window), m_visible(true), m_focused(false), m_hovered(false),
		m_pos(0,0), m_size(0,0), m_minSize(-1,-1), m_maxSize(-1,-1)
	{
		setVisible(true);
	}

	void UIComponent::recursiveDraw(NVGcontext* a_nvg) {
#ifdef DRAW_COMPONENT_BOUNDS
		if (m_hovered) {
			nvgStrokeWidth(a_nvg, 1.0f);
			nvgBeginPath(a_nvg);
			nvgRect(a_nvg, m_pos[0] - 2.0f, m_pos[1] - 2.0f, size()[0] + 2.0f, size()[1] + 2.0f);
			nvgStrokeColor(a_nvg, nvgRGBA(255, 255, 0, 225));
			nvgStroke(a_nvg);

			nvgStrokeWidth(a_nvg, 1.0f);
			nvgBeginPath(a_nvg);
			nvgRect(a_nvg, m_pos[0] - 0.5f, m_pos[1] - 0.5f, minSize()[0] + 0.5f, minSize()[1] + 0.5f);
			nvgStrokeColor(a_nvg, nvgRGBA(255, 0, 0, 200));
			nvgStroke(a_nvg);
		}
#endif


		nvgTranslate(a_nvg, m_pos[0], m_pos[1]);
		nvgSave(a_nvg);
		draw(a_nvg);
		
		for (int i = m_children.size() - 1; i >= 0;i--) {
			shared_ptr<UIComponent> child = m_children[i];
			if(child->m_visible)
				child->recursiveDraw(a_nvg);
		}
		nvgRestore(a_nvg);
		nvgTranslate(a_nvg, -m_pos[0], -m_pos[1]);
	}

	void UIComponent::addChild(UIComponent* a_newChild) {
		if (a_newChild->parent())
			return;
		shared_ptr<UIComponent> newChildPtr(a_newChild);
		addChild(newChildPtr);
	}

	void UIComponent::addChild(shared_ptr<UIComponent> a_newChild) {
		if (a_newChild->parent())
			return;
		if (find(m_children.begin(), m_children.end(), a_newChild) == m_children.end()) {
			a_newChild->m_parent = this;
			m_children.push_back(a_newChild);
			_onAddChild(a_newChild);
		}
	}

	bool UIComponent::removeChild(UIComponent* a_child) {
		for(int i=0;i<m_children.size();i++) {
			if(m_children[i].get()==a_child) {
				return removeChild(i);
			}
		}
		return false;
	}

	bool UIComponent::removeChild(int a_index) {
		if (a_index < 0 || a_index >= m_children.size())
			return false;
		if (m_window->getFocused() == m_children[a_index].get())
			m_window->clearFocus();
		m_children.erase(m_children.cbegin() + a_index);
		_onRemoveChild();
		return true;
	}

	shared_ptr<UIComponent> UIComponent::getChild(int i) {
		return m_children[i];
	}

	shared_ptr<UIComponent> UIComponent::getChild(UIComponent* a_comp) {
		for(int i=0;i<m_children.size();i++) {
			if (m_children[i].get() == a_comp)
				return m_children[i];
		}
		return nullptr;
	}

	int UIComponent::getChildIndex(UIComponent* a_comp) {
		for(int i=0;i<m_children.size();i++) {
			if (m_children[i].get() == a_comp)
				return i;
		}
		return -1;
	}

	int UIComponent::numChildren() const {
		return m_children.size();
	}

	vector<shared_ptr<UIComponent>>& UIComponent::children() {
		return m_children;
	}

	UIComponent* UIComponent::parent() const {
		return m_parent;
	}

	Vector2i UIComponent::getRelPos() const {
		return m_pos;
	}

	void UIComponent::setRelPos(const Vector2i& a_pos) {
		m_pos = a_pos;
	}

	Vector2i UIComponent::getAbsPos() const {
		return m_parent ? m_parent->getAbsPos() + getRelPos() : getRelPos();
	}

	Vector2i UIComponent::getAbsCenter() const {
		return getAbsPos() + size() / 2.0;
	}

	void UIComponent::move(const Vector2i& a_dist) {
		m_pos += a_dist;
	}

	Vector2i UIComponent::size() const {
		return m_size;
	}

	Vector2i UIComponent::minSize() const {
		return m_minSize;
	}

	Vector2i UIComponent::maxSize() const {
		return m_maxSize;
	}

	void UIComponent::grow(const Vector2i& a_amt) {
		setSize(size() + a_amt);
	}

	shared_ptr<Theme> UIComponent::theme() const {
		return m_window->theme();
	}

	bool UIComponent::visible() const {
		return m_visible;
	}

	void UIComponent::setVisible(bool a_visible) {
		if (a_visible) {
			m_visibleSize = m_size;
			setSize({ 0,0 });
		}
		else {
			setSize(m_visibleSize);
		}
		m_visible = a_visible;
	}

	bool UIComponent::contains(const Vector2i& a_pt) {
		auto pt = (a_pt - m_pos).array();			
		return (pt >= 0).all() && (pt <= size().array()).all();
	}

	UIComponent* UIComponent::findChild(const Vector2i& a_pt) {
		Vector2i relPt = a_pt - m_pos;
		for (shared_ptr<UIComponent> child : m_children) {
			if (child->visible() && child->contains(relPt))
				return child.get();
		}
		return contains(a_pt) ? this : nullptr;
	}

	UIComponent* UIComponent::findChildRecursive(const Vector2i& a_pt) {
		Vector2i relPt = a_pt - m_pos;
		for(shared_ptr<UIComponent> child : m_children) {
			if (child->visible() && child->contains(relPt))
				return child->findChildRecursive(relPt);			
		}
		return contains(a_pt) ? this : nullptr;
	}

	void UIComponent::setSize(const Vector2i& a_size) {
		Vector2i newSize = a_size;
		if (newSize[0] < 0)
			newSize[0] = m_size[0];
		if (newSize[1] < 0)
			newSize[1] = m_size[1];

		if (m_minSize[0] >= 0)
			newSize[0] = MAX(m_minSize[0], newSize[0]);
		if (m_minSize[1] >= 0)
			newSize[1] = MAX(m_minSize[1], newSize[1]);
		if (m_maxSize[0] >= 0)
			newSize[0] = MIN(m_maxSize[0], newSize[0]);
		if (m_maxSize[1] >= 0)
			newSize[1] = MIN(m_maxSize[1], newSize[1]);
		if (m_size != newSize) {
			m_size = newSize;
			_onResize();
			if (parent())
				parent()->notifyChildResized(this);
		}
	}

	void UIComponent::setMinSize_(const Vector2i& a_minSize) {
		m_minSize = a_minSize;
		setSize(m_size);
	}

	void UIComponent::setMaxSize_(const Vector2i& a_maxSize) {
		m_maxSize = a_maxSize;
		setSize(m_size);
	}

	bool UIComponent::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {			
		return false;
	}

	bool UIComponent::onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
		for (shared_ptr<UIComponent> child : m_children) {
			if (!child->visible())
				continue;
			bool hasMouse = child->contains(a_relCursor - m_pos);
			bool hadMouse = child->contains(a_relCursor - m_pos - a_diffCursor);
			if (hasMouse != hadMouse)
				child->onMouseEnter(a_relCursor - m_pos, a_diffCursor, hasMouse);
			if ((hasMouse || hadMouse) && child->onMouseMove(a_relCursor - m_pos, a_diffCursor))
				return true;
		}
		return false;
	}

	void UIComponent::onMouseEnter(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isEntering) {
		m_hovered = a_isEntering;
	}

	UIComponent* UIComponent::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
		for (shared_ptr<UIComponent> child : m_children) {
			if (!child->visible())
				continue;
			if (child->contains(a_relCursor - m_pos)) {
				UIComponent* ret = child->onMouseDown(a_relCursor - m_pos, a_diffCursor);
				if (ret) return ret;
			}
		}
		return nullptr;
	}

	bool UIComponent::onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
		for (shared_ptr<UIComponent> child : m_children) {
			if (!child->visible())
				continue;
			if (child->contains(a_relCursor - m_pos) && child->onMouseUp(a_relCursor - m_pos, a_diffCursor))
				return true;
		}
		return false;
	}

	bool UIComponent::onMouseScroll(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) {
		for (shared_ptr<UIComponent> child : m_children) {
			if (!child->visible())
				continue;
			if (child->contains(a_relCursor - m_pos) && child->onMouseScroll(a_relCursor - m_pos, a_diffCursor, a_scrollAmt))
				return true;
		}
		return false;
	}

	void UIComponent::onFocusEvent(bool a_isFocused) {
		m_focused = a_isFocused;
	}
}
