#include "UnitControl.h"
#include "UI.h"
#include "VOSIMSynth.h"

namespace syn
{
	UnitControl::UnitControl(IPlugBase* pPlug, VoiceManager* vm, Unit* unit, int x, int y, int size) :
		IControl(pPlug, {x,y,x + size,y + size}),
		m_unit(unit),
		m_size(size),
		m_minsize(0),
		m_nParams(unit->getParameterNames().size()),
		m_x(x),
		m_y(y),
		m_is_sink(false) {
		for (int i = 0; i < m_nParams; i++) {
			m_portLabels.push_back(ITextSlider(pPlug, vm, IRECT{0,0,0,0}, unit->getParent().getUnitId(unit), i));
		}
		m_ports.resize(m_nParams);
		resize(size);
	}

	void UnitControl::OnMouseDblClick(int x, int y, IMouseMod* pMod) {
		int selectedParam = getSelectedParam(x, y);
		if (selectedParam >= 0) {
			m_portLabels[selectedParam].OnMouseDblClick(x, y, pMod);
		}
	}

	void UnitControl::OnMouseDown(int x, int y, IMouseMod* pMod) {
		int selectedParam = getSelectedParam(x, y);
		if (selectedParam >= 0) {
			m_portLabels[selectedParam].OnMouseDown(x, y, pMod);
		}
	}

	void UnitControl::OnMouseUp(int x, int y, IMouseMod* pMod) {
		int selectedParam = getSelectedParam(x, y);
		if (selectedParam >= 0) {
			m_portLabels[selectedParam].OnMouseUp(x, y, pMod);
		}
	}

	UnitControl::~UnitControl() {}

	void UnitControl::move(int newx, int newy) {
		if (newx < 0 || newy < 0)
			return;
		m_x = newx;
		m_y = newy;
		mRECT.L = newx;
		mRECT.T = newy;
		mRECT.R = newx + m_size;
		mRECT.B = newy + m_size;
		SetTargetArea(mRECT);
		resize(m_size);
	}

	int UnitControl::getMinSize() const {
		return m_minsize;
	}

	void UnitControl::updateMinSize(int minsize) {
		m_minsize = max(m_minsize, minsize);
	}

	void UnitControl::resize(int newsize) {
		const vector<string>& paramNames = m_unit->getParameterNames();
		if (newsize < getMinSize())
			newsize = getMinSize();
		m_size = newsize;
		mRECT.R = m_x + m_size;
		mRECT.B = m_y + m_size;

		int portY = m_y + 20;
		int rowsize = (newsize - 30) / double(m_nParams);
		for (int i = 0; i < paramNames.size(); i++) {
			IRECT port_label_irect{m_x + 30 ,portY, m_x + newsize,portY + 10};
			m_portLabels[i].setRECT(port_label_irect);
			m_ports[i].add_rect = IRECT{m_x, portY, m_x + 10, portY + 10};
			m_ports[i].scale_rect = IRECT{m_x + 12, portY, m_x + 22, portY + 10};
			portY += rowsize;
		}
		SetTargetArea(mRECT);
		m_minsize = 10 * m_nParams + 40;
	}

	bool UnitControl::Draw(IGraphics* pGraphics) {
		Instrument* instr = static_cast<Instrument*>(&m_unit->getParent());
		// Local text palette
		IText textfmt{12, &COLOR_BLACK,"Helvetica",IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType};
		IText centertextfmt{12, &COLOR_BLACK,"Helvetica",IText::kStyleNormal,IText::kAlignCenter,0,IText::kQualityClearType};
		// Local color palette
		IColor bg_color = m_is_sink ? globalPalette[TERTIARY][2] : globalPalette[TERTIARY][1];
		IColor addport_color{150,0,255,0};
		IColor mulport_color{150,255,0,0};

		if (instr->isPrimarySource(instr->getUnitId(m_unit))) {
			IRECT paddedoutline = mRECT.GetPadded(2);
			pGraphics->DrawRect(&COLOR_WHITE, &paddedoutline);
		}

		pGraphics->FillIRect(&bg_color, &mRECT);
		pGraphics->DrawRect(&COLOR_BLACK, &mRECT);

		VOSIMSynth* vs = static_cast<VOSIMSynth*>(mPlug);
		// If this unit is the oscilloscope input
		if (vs->m_Oscilloscope->getInputId() == m_unit->getParent().getUnitId(m_unit)) {
			IRECT input_badge_irect{mRECT.R - 10,mRECT.B - 10,mRECT.R,mRECT.B};
			pGraphics->DrawIText(&textfmt, "I", &input_badge_irect);
		}
		if (vs->m_Oscilloscope->getTriggerId() == m_unit->getParent().getUnitId(m_unit)) {
			IRECT trigger_badge_irect{mRECT.R - 20,mRECT.B - 10,mRECT.R - 10,mRECT.B};
			pGraphics->DrawIText(&textfmt, "T", &trigger_badge_irect);
		}

		vector<string> paramNames = m_unit->getParameterNames();
		char strbuf[256];
		sprintf(strbuf, "%s", m_unit->getName().c_str());
		IRECT titleTextRect{m_x,m_y,m_x + m_size,m_y + 10};
		// Measure title text and resize if necessary
		pGraphics->DrawIText(&centertextfmt, strbuf, &titleTextRect, true);
		pGraphics->DrawIText(&centertextfmt, strbuf, &titleTextRect);

		updateMinSize(titleTextRect.W() + 10);

		for (int i = 0; i < paramNames.size(); i++) {
			// Draw input ports
			if (m_unit->getParam(i).canModulate()) {
				pGraphics->FillCircle(&addport_color, m_ports[i].add_rect.MW(), m_ports[i].add_rect.MH(), m_ports[i].add_rect.W() / 2, 0, true);
				pGraphics->DrawIText(&centertextfmt, "+", &m_ports[i].add_rect);
				pGraphics->FillCircle(&mulport_color, m_ports[i].scale_rect.MW(), m_ports[i].scale_rect.MH(), m_ports[i].scale_rect.W() / 2, 0, true);
				pGraphics->DrawIText(&centertextfmt, "x", &m_ports[i].scale_rect);
			}
			// Draw user control
			sprintf(strbuf, "%s", paramNames[i].c_str());
			if (!m_unit->getParam(i).isHidden()) {
				m_portLabels[i].Draw(pGraphics);
				updateMinSize(m_portLabels[i].getMinSize() + m_ports[i].add_rect.W() + m_ports[i].scale_rect.W() + 20);
			} else {
				pGraphics->DrawIText(&textfmt, strbuf, m_portLabels[i].GetRECT());
			}

			// Resize if too small
			if (m_size < getMinSize())
				resize(getMinSize());
		}
		return true;
	}

	NDPoint<2, int> UnitControl::getPos() const {
		return NDPoint<2, int>(m_x, m_y);
	}

	NDPoint<2, int> UnitControl::getPortPos(SelectedPort& port) {
		int portid = port.paramid;
		if (port.modaction == ADD) {
			return NDPoint<2, int>(m_ports[portid].add_rect.L + m_ports[portid].add_rect.W() / 2, m_ports[portid].add_rect.T + m_ports[portid].add_rect.H() / 2);
		} else if (port.modaction == SCALE) {
			return NDPoint<2, int>(m_ports[portid].scale_rect.L + m_ports[portid].scale_rect.W() / 2, m_ports[portid].scale_rect.T + m_ports[portid].scale_rect.H() / 2);
		} else {
			return NDPoint<2, int>(0, 0);
		}
	}

	NDPoint<2, int> UnitControl::getOutputPos() const {
		return NDPoint<2, int>(m_x + m_size, m_y + m_size / 2);
	}

	Unit* UnitControl::getUnit() const {
		return m_unit;
	}

	SelectedPort UnitControl::getSelectedPort(int x, int y) {
		SelectedPort selectedPort = {-1,SET};
		for (int i = 0; i < m_ports.size(); i++) {
			if (!m_unit->getParam(i).canModulate())
				continue;
			if (m_ports[i].add_rect.Contains(x, y)) {
				selectedPort.paramid = i;
				selectedPort.modaction = ADD;
			} else if (m_ports[i].scale_rect.Contains(x, y)) {
				selectedPort.paramid = i;
				selectedPort.modaction = SCALE;
			}
		}
		return selectedPort;
	}

	int UnitControl::getSelectedParam(int x, int y) {
		int selectedParam = -1;
		for (int i = 0; i < m_portLabels.size(); i++) {
			if (m_unit->getParam(i).isHidden())
				continue;
			if (m_portLabels[i].GetRECT()->Contains(x, y)) {
				selectedParam = i;
			}
		}
		return selectedParam;
	}
}

