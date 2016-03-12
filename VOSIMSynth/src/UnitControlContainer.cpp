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

#include "UnitControlContainer.h"
#include <DSPMath.h>

namespace syn
{
	UnitControlContainer::UnitControlContainer(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, UnitControl* a_unitControl, int a_unitId, int x, int y) :
		m_voiceManager(a_vm),
		m_unitControl(a_unitControl),
		m_isSelected(false),
		m_size(0, 0),
		m_minSize(0, 0),
		m_plug(a_plug),
		m_isControlClicked(false)
	{
		UnitControlContainer::setUnitId(a_unitId);
		UnitControlContainer::move(x, y);
	}

	void UnitControlContainer::onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod) {
		if (m_unitControl->isHit(a_x, a_y)) {
			m_unitControl->onMouseDblClick(a_x, a_y, a_mouseMod);
		}
	}

	bool UnitControlContainer::onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod) {
		if (m_unitControl->isHit(a_x, a_y) && !m_isSelected) {
			m_isControlClicked = true;
			m_unitControl->onMouseDown(a_x, a_y, a_mouseMod);
			return true;
		}
		m_isControlClicked = false;
		return false;
	}

	bool UnitControlContainer::onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod) {
		m_isControlClicked = false;
		if (m_isControlClicked && !m_isSelected) {
			m_unitControl->onMouseUp(a_x, a_y, a_mouseMod);
			return true;
		}
		return false;
	}

	void UnitControlContainer::onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod) {
		if (m_isControlClicked && !m_isSelected) { // modify control
			m_unitControl->onMouseDrag(a_x, a_y, a_dX, a_dY, a_mouseMod);
		} else if (a_mouseMod->L) { 
			if (a_mouseMod->C) { // resize
				resize(m_size + NDPoint<2, int>(a_dX, a_dY));
			}
			else { // move
				NDPoint<2, int> newUnitPos = getPos() + NDPoint<2, int>(a_dX, a_dY);
				move(newUnitPos[0], newUnitPos[1]);
			}
		}
	}

	void UnitControlContainer::onMouseOver(int a_x, int a_y, IMouseMod* a_mouseMod) {
		if(m_unitControl->isHit(a_x,a_y)) {
			m_unitControl->onMouseOver(a_x, a_y, a_mouseMod);
		}
	}

	void UnitControlContainer::onMouseWheel(int a_x, int a_y, IMouseMod* a_mouseMod, int a_d) {
		if(m_unitControl->isHit(a_x, a_y)) {
			m_unitControl->onMouseWheel(a_x, a_y, a_mouseMod, a_d);
		}
	}

	void UnitControlContainer::move(int a_newx, int a_newy) {
		if (a_newx < 0 || a_newy < 0)
			return;
		m_rect.L = a_newx;
		m_rect.T = a_newy;
		m_rect.R = a_newx + m_size[0];
		m_rect.B = a_newy + m_size[1];
		resize(m_size);
	}

	NDPoint<2, int> UnitControlContainer::getMinSize() const {
		NDPoint<2,int> controlMinSize = m_unitControl->getMinSize();
		controlMinSize[0] = MAX(controlMinSize[0], m_titleSize[0]);
		controlMinSize[1] = MAX(controlMinSize[1], m_minSize[1]);
		controlMinSize[0] += m_minSize[0];
		controlMinSize[1] += m_titleSize[1];
		return controlMinSize;
	}

	void UnitControlContainer::_resetMinSize() {
		int nInputs = m_voiceManager->getUnit(m_unitId).getNumInputs();
		int nOutputs = m_voiceManager->getUnit(m_unitId).getNumOutputs();
		int lastInPortY = getPortPos({ UnitPortVector::Input, nInputs - 1 })[1];
		int lastOutPortY = getPortPos({ UnitPortVector::Output, nOutputs - 1 })[1];
		m_minSize[0] = 2 * (2*c_portPad + m_portSize[0]);
		m_minSize[1] = MAX(lastInPortY - m_rect.T, lastOutPortY - m_rect.T);
	}

	void UnitControlContainer::_updateMinSize(NDPoint<2, int> a_newMinSize) {
		m_minSize[0] = MAX(m_minSize[0], a_newMinSize[0]);
		m_minSize[1] = MAX(m_minSize[1], a_newMinSize[1]);
	}

	void UnitControlContainer::resize(NDPoint<2, int> a_newSize) {
		m_size[0] = MAX(getMinSize()[0], a_newSize[0]);
		m_size[1] = MAX(getMinSize()[1], a_newSize[1]);
		m_rect.R = m_rect.L + m_size[0];
		m_rect.B = m_rect.T + m_size[1];
		m_unitControl->move({ m_rect.L + m_portSize[0] + 2*c_portPad, m_rect.T + 10 });
		m_unitControl->resize({ m_size[0] - 2 * (m_portSize[0] + 2*c_portPad), m_size[1] - c_edgePad - m_titleSize[1] });
	}

	bool UnitControlContainer::draw(IGraphics* pGraphics) {
		// Local text palette
		IText textfmt{ 12, &COLOR_BLACK,"Arial",IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType };
		IText centertextfmt{ 12, &COLOR_BLACK,"Arial",IText::kStyleNormal,IText::kAlignCenter,0,IText::kQualityClearType };
		// Local color palette
		IColor outline_color = { 255,0,0,0 };
		IColor selected_outline_color = { 255,200,200,0 };
		IColor bg_color = { 235,255,255,255 };
		IColor input_port_color{ 255,0,255,0 };
		IColor output_port_color{ 255,0,0,255 };

		pGraphics->FillIRect(&bg_color, &m_rect);
		pGraphics->DrawRect(&outline_color, &m_rect);
		if(m_isSelected) {
			IRECT outlineRect = m_rect.GetPadded(1);
			pGraphics->DrawRect(&selected_outline_color, &outlineRect);
		}
		

		int nInputPorts = m_voiceManager->getUnit(m_unitId).getNumInputs();
		int nOutputPorts = m_voiceManager->getUnit(m_unitId).getNumOutputs();

		char strbuf[256];
		snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getName().c_str());
		IRECT titleTextRect{ m_rect.L,m_rect.T,m_rect.L + m_size[0],m_rect.T + 10 };
		// Measure title text and resize if necessary
		pGraphics->DrawIText(&centertextfmt, strbuf, &titleTextRect, true);
		pGraphics->DrawIText(&centertextfmt, strbuf, &titleTextRect);
		titleTextRect = titleTextRect.GetHPadded(1);
		pGraphics->DrawRect(&COLOR_BLACK, &titleTextRect);

		_resetMinSize();
		m_titleSize = {titleTextRect.W(),  titleTextRect.H() };
		m_portSize = { 5,5 };

		IRECT portRect;
		IRECT measureRect;
		// Measure input ports
		for (int i = 0; i<nInputPorts; i++) {
			snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getInputName(i).c_str());
			pGraphics->DrawIText(&centertextfmt, strbuf, &measureRect, true);
			m_portSize[1] = MAX(m_portSize[1], measureRect.H());
			m_portSize[0] = MAX(m_portSize[0], measureRect.W());
		}
		// Measure output ports
		for (int i = 0; i<nOutputPorts; i++) {
			snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getOutputName(i).c_str());
			pGraphics->DrawIText(&centertextfmt, strbuf, &measureRect, true);
			m_portSize[1] = MAX(m_portSize[1], measureRect.H());
			m_portSize[0] = MAX(m_portSize[0], measureRect.W());
		}
		// Draw input ports
		for (int i = 0; i < nInputPorts; i++) {
			snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getInputName(i).c_str());
			portRect = getPortRect({ UnitPortVector::Input, i });
			pGraphics->DrawRect(&input_port_color, &portRect);
			pGraphics->DrawIText(&centertextfmt, strbuf, &portRect);
		}
		// Draw output ports
		for (int i = 0; i < nOutputPorts; i++) {
			snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getOutputName(i).c_str());
			portRect = getPortRect({ UnitPortVector::Output, i });
			pGraphics->DrawRect(&output_port_color, &portRect);
			pGraphics->DrawIText(&centertextfmt, strbuf, &portRect);
		}

		m_unitControl->draw(pGraphics);

		// Resize if too small
		resize(m_size);
		return true;
	}

	NDPoint<2, int> UnitControlContainer::getPos() const {
		return NDPoint<2, int>(m_rect.L, m_rect.T);
	}

	NDPoint<2, int> UnitControlContainer::getSize() const {
		return m_size;
	}

	NDPoint<2, int> UnitControlContainer::getPortPos(UnitPortVector a_portVector) const {
		int portId = a_portVector.id;
		int x = 0, y = 0;
		switch (a_portVector.type) {
		case UnitPortVector::Input:
			x = m_rect.L + m_portSize[0] / 2 + c_portPad;
			y = m_rect.T + c_portPad + m_portSize[1]*portId + m_portSize[1] / 2;
			break;
		case UnitPortVector::Output:
			x = m_rect.L + m_rect.W() - m_portSize[0] / 2 - c_portPad;
			y = m_rect.T + c_portPad + m_portSize[1] * portId + m_portSize[1] / 2;
			break;
		case UnitPortVector::Null:
		default:
			break;
		}
		return NDPoint<2, int>(x, y);
	}

	IRECT UnitControlContainer::getPortRect(UnitPortVector a_portVector) const {
		NDPoint<2, int> portPos = getPortPos(a_portVector);
		return IRECT{ portPos[0] - m_portSize[0] / 2,portPos[1] - m_portSize[1] / 2,portPos[0] + m_portSize[0] / 2,portPos[1] + m_portSize[1] / 2 };
	}

	UnitPortVector UnitControlContainer::getSelectedPort(int x, int y) const {
		int nInputPorts = m_voiceManager->getUnit(m_unitId).getNumInputs();
		int nOutputPorts = m_voiceManager->getUnit(m_unitId).getNumOutputs();
		IRECT portRect;
		for (int i = 0; i < nInputPorts; i++) {
			portRect = getPortRect({ UnitPortVector::Input, i });
			if (portRect.Contains(x, y)) {
				return{ UnitPortVector::Input, i };
			}
		}
		for (int i = 0; i < nOutputPorts; i++) {
			portRect = getPortRect({ UnitPortVector::Output, i });
			if (portRect.Contains(x, y)) {
				return{ UnitPortVector::Output, i };
			}
		}
		return{ UnitPortVector::Null, 0 };
	}

	void UnitControlContainer::setUnitId(int a_newUnitId)
	{
		m_unitId = a_newUnitId;
		m_unitControl->setUnitId(a_newUnitId);
	}

	int UnitControlContainer::getUnitId() const
	{
		return m_unitId;
	}

	void UnitControlContainer::setIsSelected(bool a_isSelected) {
		m_isSelected = a_isSelected;
	}

	bool UnitControlContainer::isHit(int a_x, int a_y) {
		return m_rect.Contains(a_x, a_y);
	}

	bool UnitControlContainer::isHit(IRECT& a_rect) {
		return a_rect.Intersects(&m_rect);
	}
}
