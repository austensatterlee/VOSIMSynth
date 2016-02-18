#include "UnitControlContainer.h"
#include <DSPMath.h>

namespace syn
{
	UnitControlContainer::UnitControlContainer(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, UnitControl* a_unitControl, int a_unitId, int x, int y) :
		m_voiceManager(a_vm),
		m_unitControl(a_unitControl),
		m_size(0, 0),
		m_minSize(0, 0),
		m_plug(a_plug),
		m_isControlClicked(false)
	{
		setUnitId(a_unitId);
		move(x, y);
	}

	void UnitControlContainer::onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod) {
		if (m_unitControl->isHit(a_x, a_y)) {
			m_unitControl->onMouseDblClick(a_x, a_y, a_mouseMod);
		}
	}

	void UnitControlContainer::onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod) {
		if (m_unitControl->isHit(a_x, a_y)) {
			m_isControlClicked = true;
			m_unitControl->onMouseDown(a_x, a_y, a_mouseMod);
		}else {
			m_isControlClicked = false;
		}
	}

	void UnitControlContainer::onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod) {
		if (m_isControlClicked)
			m_unitControl->onMouseUp(a_x, a_y, a_mouseMod);
		m_isControlClicked = false;
	}

	void UnitControlContainer::onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod) {
		if (m_isControlClicked) { // modify control
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
		return m_minSize + m_unitControl->getMinSize();
	}

	void UnitControlContainer::_resetMinSize() {
		int nInputs = m_voiceManager->getUnit(m_unitId).getNumInputs();
		int nOutputs = m_voiceManager->getUnit(m_unitId).getNumOutputs();
		m_minSize[1] = 15+MAX(c_portSize[1] * nOutputs, c_portSize[1] * nInputs);
		m_minSize[0] = 2 * (5 + c_portSize[0]);
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
		m_unitControl->move({ m_rect.L + c_portSize[0] + 5, m_rect.T + 10 });
		m_unitControl->resize({ m_size[0] - 2 * (c_portSize[0] + 5), m_size[1] - 10 });
	}

	bool UnitControlContainer::draw(IGraphics* pGraphics) {
		// Local text palette
		IText textfmt{ 12, &COLOR_BLACK,"Helvetica",IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType };
		IText centertextfmt{ 12, &COLOR_BLACK,"Helvetica",IText::kStyleNormal,IText::kAlignCenter,0,IText::kQualityClearType };
		// Local color palette
		IColor outline_color = { 255,0,0,0 };
		IColor bg_color = { 255,255,255,255 };
		IColor input_port_color{ 255,0,255,0 };
		IColor output_port_color{ 255,0,0,255 };

		pGraphics->FillIRect(&bg_color, &m_rect);
		pGraphics->DrawRect(&outline_color, &m_rect);

		int nInputPorts = m_voiceManager->getUnit(m_unitId).getNumInputs();
		int nOutputPorts = m_voiceManager->getUnit(m_unitId).getNumOutputs();

		char strbuf[256];
		snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getName().c_str());
		IRECT titleTextRect{ m_rect.L,m_rect.T,m_rect.L + m_size[0],m_rect.T + 10 };
		// Measure title text and resize if necessary
		pGraphics->DrawIText(&centertextfmt, strbuf, &titleTextRect, true);
		pGraphics->DrawIText(&centertextfmt, strbuf, &titleTextRect);

		_resetMinSize();
		_updateMinSize({ titleTextRect.W(), 0 });

		IRECT portRect;
		IRECT measureRect;
		// Draw input ports
		for (int i = 0; i<nInputPorts; i++) {
			snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getInputName(i).c_str());
			pGraphics->DrawIText(&centertextfmt, strbuf, &measureRect, true);
			c_portSize[1] = MAX(c_portSize[1], measureRect.H());
			c_portSize[0] = MAX(c_portSize[0], measureRect.W() + 2);

			portRect = getPortRect({ UnitPortVector::Input, i });
			pGraphics->DrawRect(&input_port_color, &portRect);
			pGraphics->DrawIText(&centertextfmt, strbuf, &portRect);
		}
		// Draw output ports
		for (int i = 0; i<nOutputPorts; i++) {
			snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getOutputName(i).c_str());
			pGraphics->DrawIText(&centertextfmt, strbuf, &measureRect, true);
			c_portSize[1] = MAX(c_portSize[1], measureRect.H());
			c_portSize[0] = MAX(c_portSize[0], measureRect.W() + 2);

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
		int nPorts;
		int x = 0, y = 0;
		switch (a_portVector.type) {
		case UnitPortVector::Input:
			nPorts = m_voiceManager->getUnit(m_unitId).getNumInputs();
			x = m_rect.L + c_portSize[0] / 2 + 1;
			y = m_rect.T + m_rect.H() / nPorts*portId + c_portSize[1] / 2;
			y += (m_rect.H() - m_rect.H() / (nPorts + 1)*nPorts) / nPorts; // center ports vertically
			break;
		case UnitPortVector::Output:
			nPorts = m_voiceManager->getUnit(m_unitId).getNumOutputs();
			x = m_rect.L + m_rect.W() - c_portSize[0] / 2 - 1;
			y = m_rect.T + m_rect.H() / nPorts*portId + c_portSize[1] / 2;
			y += (m_rect.H() - m_rect.H() / (nPorts + 1)*nPorts) / nPorts; // center ports vertically
			break;
		case UnitPortVector::Null:
		default:
			break;
		}
		return NDPoint<2, int>(x, y);
	}

	IRECT UnitControlContainer::getPortRect(UnitPortVector a_portVector) const {
		NDPoint<2, int> portPos = getPortPos(a_portVector);
		return IRECT{ portPos[0] - c_portSize[0] / 2,portPos[1] - c_portSize[1] / 2,portPos[0] + c_portSize[0] / 2,portPos[1] + c_portSize[1] / 2 };
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

	bool UnitControlContainer::isHit(int a_x, int a_y) {
		return m_rect.Contains(a_x, a_y);
	}
}
