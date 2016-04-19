#include "UIComponent.h"

namespace syn
{
	UIComponent::UIComponent(VOSIMWindow* a_window, UIComponent* a_parent):
		m_parent(nullptr), m_window(a_window), m_visible(true), m_focused(false), m_theme(nullptr), m_pos(0,0)
	{
		if (a_parent) {
			a_parent->addChild(this);
			m_theme = a_parent->m_theme;
		}
	}

	void UIComponent::recursiveDraw(NVGcontext* a_nvg) {
		nvgStrokeWidth(a_nvg, 1.0f);
		nvgBeginPath(a_nvg);
		nvgRect(a_nvg, m_pos[0] - 0.5f, m_pos[1] - 0.5f, m_size[0] + 1, m_size[1] + 1);
		nvgStrokeColor(a_nvg, nvgRGBA(100, 0, 0, 255));
		nvgStroke(a_nvg);

		draw(a_nvg);

		nvgTranslate(a_nvg, m_pos[0], m_pos[1]);
		
		for (int i = m_children.size() - 1; i >= 0;i--) {
			shared_ptr<UIComponent> child = m_children[i];
			if(child->m_visible)
				child->recursiveDraw(a_nvg);
		}
		nvgTranslate(a_nvg, -m_pos[0], -m_pos[1]);
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

	shared_ptr<UIComponent> UIComponent::getChild(int i) {
		return m_children[i];
	}

	bool UIComponent::contains(const Vector2i& a_pt) {
		auto pt = (a_pt - m_pos).array();			
		return (pt >= 0).all() && (pt <= m_size.array()).all();
	}

	UIComponent* UIComponent::findChild(const Vector2i& a_pt) {
		Vector2i relPt = a_pt - m_pos;
		for(shared_ptr<UIComponent> child : m_children) {
			if (!child->visible() || !child->contains(relPt))
				continue;
			if (child->numChildren()) {
				return findChild(relPt);
			} else {
				return child.get();
			}
		}
		return contains(relPt) ? this : nullptr;
	}

	Vector2i UIComponent::calcAutoSize() const {
		return{ 0,0 };
	}

	bool UIComponent::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
			
		return false;
	}

	bool UIComponent::onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
		for (shared_ptr<UIComponent> child : m_children) {
			if (!child->visible())
				continue;
			if (child->contains(a_relCursor - m_pos) && child->onMouseMove(a_relCursor - m_pos, a_diffCursor))
				return true;
		}
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
		m_focused = true;
	}
}
