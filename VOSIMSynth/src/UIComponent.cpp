#include "UIComponent.h"

namespace syn
{
	UIComponent::UIComponent(VOSIMWindow* a_window):
		m_parent(nullptr), m_window(a_window), m_visible(true), m_focused(false), m_autoSize(true), m_pos(0,0), m_size(0,0)
	{
	}

	void UIComponent::recursiveDraw(NVGcontext* a_nvg) {
#ifdef DRAW_COMPONENT_BOUNDS
		nvgStrokeWidth(a_nvg, 1.0f);
		nvgBeginPath(a_nvg);
		nvgRect(a_nvg, m_pos[0] - 0.5f, m_pos[1] - 0.5f, size()[0] + 1, size()[1] + 1);
		nvgStrokeColor(a_nvg, nvgRGBA(100, 0, 0, 255));
		nvgStroke(a_nvg);
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

		if(m_autoSize) {
			setSize(calcAutoSize());
		}
	}

	void UIComponent::addChild(UIComponent* a_newChild) {
		if (a_newChild->parent())
			return;
		shared_ptr<UIComponent> a_newChildPtr(a_newChild);
		if (find(m_children.begin(), m_children.end(), a_newChildPtr) == m_children.end()) {
			a_newChild->m_parent = this;
			m_children.push_back(a_newChildPtr);
		}
	}

	bool UIComponent::removeChild(UIComponent* a_child) {
		for(int i=0;i<m_children.size();i++) {
			if(m_children[i].get()==a_child) {
				if (m_window->getFocused() == a_child)
					m_window->clearFocus();
				m_children.erase(m_children.cbegin() + i);
				return true;
			}
		}
		return false;
	}

	bool UIComponent::removeChild(int a_index) {
		if (a_index < 0 || a_index >= m_children.size())
			return false;
		m_children.erase(m_children.cbegin() + a_index);
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

	bool UIComponent::contains(const Vector2i& a_pt) {
		auto pt = (a_pt - m_pos).array();			
		return (pt >= 0).all() && (pt <= size().array()).all();
	}

	UIComponent* UIComponent::findChild(const Vector2i& a_pt) {
		Vector2i relPt = a_pt - m_pos;
		for(shared_ptr<UIComponent> child : m_children) {
			if (child->visible() && child->contains(relPt))
				return child->findChild(relPt);			
		}
		return contains(a_pt) ? this : nullptr;
	}

	Vector2i UIComponent::calcAutoSize() const {
		return m_size;
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
			if (hasMouse && child->onMouseMove(a_relCursor - m_pos, a_diffCursor))
				return true;
		}
		return false;
	}

	bool UIComponent::onMouseEnter(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isEntering) {
		m_hovered = a_isEntering;
		return false;
	}

	bool UIComponent::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
		for (shared_ptr<UIComponent> child : m_children) {
			if (!child->visible())
				continue;
			if (child->contains(a_relCursor - m_pos) && child->onMouseDown(a_relCursor - m_pos, a_diffCursor))
				return true;
		}
		return false;
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
