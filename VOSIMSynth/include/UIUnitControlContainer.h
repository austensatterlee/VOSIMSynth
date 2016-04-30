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
#include <VoiceManager.h>
#include "UIUnitControl.h"
#include "UIButton.h"

namespace syn
{
	class UIUnitPort : public UIComponent
	{
	public:
		UIUnitPort(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, int a_portNum, bool a_isInput)
			: UIComponent{ a_window }, m_vm(a_vm), m_unitId(a_unitId), m_portNum(a_portNum), m_isInput(a_isInput),m_autoWidth(0) {}
		bool onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		Vector2i calcAutoSize() const override;
		int getUnitId() const { return m_unitId; }
		int getPortId() const { return m_portNum; }
		bool isInput() const { return m_isInput; }
	protected:
		void draw(NVGcontext* a_nvg) override;
	protected:
		VoiceManager* m_vm;
		int m_unitId;
		int m_portNum;
		bool m_isInput;
		int m_autoWidth;

		bool m_isDragging = false;

		int m_textHeight = 14;
	};

	class UIUnitControlContainer : public UIWindow
	{
	friend class UICircuitPanel;
	public:
		UIUnitControlContainer(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, UIUnitControl* a_unitControl);
		Vector2i calcAutoSize() const override;
		void onResize() override;
		int getUnitId() const { return m_unitId; }
		UIUnitPort* getSelectedInPort(const Vector2i& a_relPos) const;
		UIUnitPort* getSelectedOutPort(const Vector2i& a_relPos) const;
		const vector<UIUnitPort*>& getInPorts() const { return m_inPorts; }
		const vector<UIUnitPort*>& getOutPorts() const { return m_outPorts; }
		bool onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		void close();
	protected:
		void draw(NVGcontext* a_nvg) override;
	protected:
		VoiceManager* m_vm;
		int m_unitId;
		UIUnitControl* m_unitControl;
		vector<UIUnitPort*> m_inPorts;
		vector<UIUnitPort*> m_outPorts;
		UIButton* m_closeButton;

		int m_inWidth;
		int m_outWidth;
	};

}
#endif