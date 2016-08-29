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
*  \file UIUnitContainer.h
*  \brief Representation of a Unit on the CircuitPanel.
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UIUNITCONTROLCONTAINER__
#define __UIUNITCONTROLCONTAINER__
#include "UIWindow.h"
#include <Unit.h>

namespace synui
{

	class UIUnitContainer: public virtual UIComponent
	{
		friend class UICircuitPanel;
	public:
		UIUnitContainer(MainWindow* a_window, UICircuitPanel* a_circuitPanel, syn::VoiceManager* a_vm, UIUnitControl* a_unitCtrl);

		int getUnitId() const {
			return m_unitId;
		}

		shared_ptr<UIUnitControl> getUnitControl() const {
			return m_unitCtrl;
		}

		virtual UIUnitPort* getSelectedInPort(const UICoord& a_relPos) const;
		virtual UIUnitPort* getSelectedOutPort(const UICoord& a_absPt) const;

		const vector<UIUnitPort*>& getInPorts() const {
			return m_inPorts;
		}

		const vector<UIUnitPort*>& getOutPorts() const {
			return m_outPorts;
		}

		void close() const;

		bool onMouseMove(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;

		void makeSelected(bool a_isSelected);

		bool isSelected() const;
	protected:
		void draw(NVGcontext* a_nvg) override;
	protected:
		vector<UIUnitPort*> m_inPorts;
		vector<UIUnitPort*> m_outPorts;
		syn::VoiceManager* m_vm;
		UICircuitPanel* m_circuitPanel;
		int m_unitId;
		bool m_isSelected;
		shared_ptr<UIUnitControl> m_unitCtrl;
	};

	class UIDefaultUnitContainer : public UIUnitContainer
	{
	public:
		UIDefaultUnitContainer(MainWindow* a_window, UICircuitPanel* a_circuitPanel, syn::VoiceManager* a_vm, UIUnitControl* a_unitCtrl);
		UIComponent* onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
		bool onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;
	private:
		void _onResize() override;
	protected:
		UIButton* m_closeButton;
		UIRow* m_titleRow;
		UICol* m_col;
		UIRow* m_portRow;
		UICol* m_portCols[2];
	};

	class UIInputUnitContainer : public UIUnitContainer
	{
	public:
		UIInputUnitContainer(MainWindow* a_window, UICircuitPanel* a_circuitPanel, syn::VoiceManager* a_vm, UIUnitControl* a_unitCtrl);
	private:
		UIRow* m_titleRow;
		UICol* m_col;
		UIRow* m_portRow;
		UICol* m_portCol;
	};

	class UIOutputUnitContainer : public UIUnitContainer
	{
	public:
		UIOutputUnitContainer(MainWindow* a_window, UICircuitPanel* a_circuitPanel, syn::VoiceManager* a_vm, UIUnitControl* a_unitCtrl);
	private:
		UIRow* m_titleRow;
		UICol* m_col;
		UIRow* m_portRow;
		UICol* m_portCol;
	};
}
#endif
