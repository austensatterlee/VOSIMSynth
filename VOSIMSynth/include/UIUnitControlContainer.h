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

/**
*  \file UIUnitControlContainer.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UIUNITCONTROLCONTAINER__
#define __UIUNITCONTROLCONTAINER__
#include "UIWindow.h"

namespace syn
{
	class UIUnitControlContainer : public UIWindow
	{
	private:
		friend class UICircuitPanel;
	public:
		UIUnitControlContainer(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, UIUnitControl* a_unitControl);

		int getUnitId() const {
			return m_unitId;
		}

		virtual UIUnitPort* getSelectedInPort(const Vector2i& a_relPos) const;
		virtual UIUnitPort* getSelectedOutPort(const Vector2i& a_relPos) const;

		const vector<UIUnitPort*>& getInPorts() const {
			return m_inPorts;
		}

		const vector<UIUnitPort*>& getOutPorts() const {
			return m_outPorts;
		}

		UIComponent* onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
		void close() const;
	protected:
		void draw(NVGcontext* a_nvg) override;
	protected:
		VoiceManager* m_vm;
		int m_unitId;
		UIUnitControl* m_unitControl;
		vector<UIUnitPort*> m_inPorts;
		vector<UIUnitPort*> m_outPorts;
		UIButton* m_closeButton;

		UIRow* m_row;
		UICol* m_cols[2];
	};
}
#endif
