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

/** \file UIComponent.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UICOMPONENT__
#define __UICOMPONENT__

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <thread>
#includue "UIComponent.h"
#include <NDPoint.h>

#include "nanovg.h"

namespace syn
{
	template <typename T>
	struct Timestamped
	{
		T data;
		sf::Time time;
	};

	class UIComponent
	{
	public:

		UIComponent() :
			m_parent(nullptr), m_visible(false), m_focused(false) {}

		UIComponent(const UIComponent& a_other);

		virtual ~UIComponent() {}

		void recursiveDraw(NVGcontext* a_nvg) {
			nvgStrokeWidth(a_nvg, 1.0f);
			nvgBeginPath(a_nvg);
			nvgRect(a_nvg, m_pos[0] - 0.5f, m_pos[1] - 0.5f, m_size[0] + 1, m_size[1] + 1);
			nvgStrokeColor(a_nvg, nvgRGBA(255, 0, 0, 255));
			nvgStroke(a_nvg);

			draw(a_nvg);

			nvgTranslate(a_nvg, m_pos[0], m_pos[1]);
			for (std::shared_ptr<UIComponent> child : m_children) {
				child->recursiveDraw(a_nvg);
			}
			nvgTranslate(a_nvg, -m_pos[0], -m_pos[1]);
		}

		void handleEvent(sf::Event a_event) { }

		void addChild(UIComponent* a_newChild) {
			if (a_newChild->getParent())
				return;
			std::shared_ptr<UIComponent> a_newChildPtr(a_newChild);
			if (std::find(m_children.begin(), m_children.end(), a_newChildPtr) == m_children.end()) {
				a_newChild->m_parent = this;
				m_children.push_back(a_newChildPtr);
			}
		}

		std::shared_ptr<UIComponent> getChild(int i) {
			if (i >= m_children.size())
				return nullptr;
			return m_children[i];
		}

		int numChildren() const {
			return m_children.size();
		}

		UIComponent* getParent() const {
			return m_parent;
		}

		NDPoint<2, int> getPos() const {
			return m_pos;
		}

		void setPos(const NDPoint<2, int>& a_pos) {
			m_pos = a_pos;
		}

		void move(const NDPoint<2, int>& a_dist) {
			m_pos += a_dist;
		}

		NDPoint<2, int> getSize() const {
			return m_size;
		}

		void setSize(const NDPoint<2, int>& a_size) {
			m_size = a_size;
		}

		void grow(const NDPoint<2, int>& a_amt);

	protected:
		virtual void draw(NVGcontext* a_nvg) {};

	protected:
		UIComponent* m_parent;
		std::vector<std::shared_ptr<UIComponent>> m_children;

		bool m_visible, m_focused;
		NDPoint<2, int> m_pos, m_size, m_fixedSize;
	};
};
#endif