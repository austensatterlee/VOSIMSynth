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
 *  \file UIStackedComponent.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 07/2016
 */

#ifndef __UISTACKEDCOMPONENT__
#define __UISTACKEDCOMPONENT__
#include "UIComponent.h"

namespace synui
{
	class UIStackedComponent : public UIComponent {
	public:
		UIStackedComponent(MainWindow* parent);

		void setSelectedIndex(int index);
		int selectedIndex() const;

		virtual void performLayout(NVGcontext* ctx) override;
		Vector2i calcMinSize(NVGcontext* ctx) const;
		void addChild(int index, UIComponent* child);

		int childIndex(UIComponent* a_comp) const;

		void notifyChildResized(UIComponent* a_child) override;
	private:
		void _onResize() override;
	private:
		int m_selectedIndex = -1;
		map<int, UIComponent*> m_childMap;
		map<UIComponent*, int> m_revChildMap;
	};
}

#endif
