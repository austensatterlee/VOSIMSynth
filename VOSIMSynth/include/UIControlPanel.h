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
 *  \file UIControlPanel.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 06/2016
 */

#ifndef __UICONTROLPANEL__
#define __UICONTROLPANEL__
#include "UIComponent.h"

namespace synui {
	class UIControlPanel : public UIComponent
	{
	public:
		UIControlPanel(MainWindow* a_window, UITabWidget* a_ctrlPanelTabHeader = nullptr);
		void showUnitControl(UIUnitContainer* a_unitCtrl);
		void clearUnitControl();
		UIUnitContainer* getCurrentUnitContainer() const;
		UIUnitControl* getCurrentUnitControl() const;

	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		void _onResize() override;
	private:
		UIScrollPanel* m_scrollPanel;
		UIUnitContainer* m_currUnitContainer;
		UITabWidget* m_parentTabComponent;
	};
}

#endif
