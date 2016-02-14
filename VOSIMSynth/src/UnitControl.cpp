#include "UnitControl.h"

namespace syn
{
	UnitControl::UnitControl(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int x, int y) :
		IControl(a_plug, {x,y,x,y}),
        m_voiceManager(a_vm),
		m_size(0,0),
		m_minSize(0,0),
		m_x(x),
		m_y(y)
	{
		setUnitId(a_unitId);
	}

	UnitControl::~UnitControl() {
        m_paramControls.clear();
    }

	void UnitControl::OnMouseDblClick(int x, int y, IMouseMod* pMod) {
		int selectedParam = getSelectedParam(x, y);
		if (selectedParam >= 0) {
			m_paramControls[selectedParam].OnMouseDblClick(x, y, pMod);
		}
	}

	void UnitControl::OnMouseDown(int x, int y, IMouseMod* pMod) {
		int selectedParam = getSelectedParam(x, y);
		if (selectedParam >= 0) {
			m_paramControls[selectedParam].OnMouseDown(x, y, pMod);
		}
	}

	void UnitControl::OnMouseUp(int x, int y, IMouseMod* pMod) {
		int selectedParam = getSelectedParam(x, y);
		if (selectedParam >= 0) {
			m_paramControls[selectedParam].OnMouseUp(x, y, pMod);
		}
	}

	void UnitControl::move(int a_newx, int a_newy) {
		if (a_newx < 0 || a_newy < 0)
			return;
		m_x = a_newx;
		m_y = a_newy;
		mRECT.L = a_newx;
		mRECT.T = a_newy;
		mRECT.R = a_newx + m_size[0];
		mRECT.B = a_newy + m_size[1];
		SetTargetArea(mRECT);
		resize(m_size);
	}

	NDPoint<2,int> UnitControl::getMinSize() const {
		return m_minSize;
	}

	void UnitControl::_updateMinSize(NDPoint<2, int> minsize) {
		m_minSize[0] = MAX(minsize[0] + 2*c_portSize[0], m_minSize[0]);
		m_minSize[1] = MAX(minsize[1], m_minSize[1]);
	}

	void UnitControl::_resetMinSize(){
		int nParams = m_voiceManager->getUnit(m_unitId).getNumParameters();
		m_minSize[1] = 10 * nParams + 40;
		m_minSize[0] = m_minSize[1] + 2 * c_portSize[0];

	}

	void UnitControl::resize(NDPoint<2, int> a_newSize) {
        int nParams = m_paramControls.size();
		if (a_newSize[0] < getMinSize()[0])
			a_newSize[0] = getMinSize()[0];
		if (a_newSize[1] < getMinSize()[1])
			a_newSize[1] = getMinSize()[1];
		m_size = a_newSize;
		mRECT.R = m_x + m_size[0];
		mRECT.B = m_y + m_size[1];
        if(nParams) {
            int portY = m_y + 20;
            int rowsize = (a_newSize[1] - 20) / nParams;
            for (int i = 0 ; i < nParams ; i++) {
                IRECT param_rect{m_x + c_portSize[0] + 5, portY, m_x + a_newSize[0] - c_portSize[0] - 5, portY + rowsize};
                m_paramControls[i].setRECT(param_rect);
                portY += rowsize;
            }
        }
		SetTargetArea(mRECT);
	}

	bool UnitControl::Draw(IGraphics* pGraphics) {
		// Local text palette
		IText textfmt{12, &COLOR_BLACK,"Helvetica",IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType};
		IText centertextfmt{12, &COLOR_BLACK,"Helvetica",IText::kStyleNormal,IText::kAlignCenter,0,IText::kQualityClearType};
		// Local color palette
        IColor outline_color = {255,0,0,0};
		IColor bg_color = {255,255,255,255};
		IColor input_port_color{255,0,255,0};
        IColor output_port_color{255,0,0,255};

		pGraphics->FillIRect(&bg_color, &mRECT);
		pGraphics->DrawRect(&outline_color, &mRECT);

        int nParams = m_paramControls.size();
        int nInputPorts = m_voiceManager->getUnit(m_unitId).getNumInputs();
        int nOutputPorts = m_voiceManager->getUnit(m_unitId).getNumOutputs();

		char strbuf[256];
		snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getName().c_str());
		IRECT titleTextRect{m_x,m_y,m_x + m_size[0],m_y + 10};
		// Measure title text and resize if necessary
		pGraphics->DrawIText(&centertextfmt, strbuf, &titleTextRect, true);
		pGraphics->DrawIText(&centertextfmt, strbuf, &titleTextRect);

		_resetMinSize();
		_updateMinSize({ titleTextRect.W() + 10,10 });

        IRECT portRect;
		IRECT measureRect;
        // Draw input ports
        for (int i=0;i<nInputPorts;i++){
			snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getInputName(i).c_str());
			pGraphics->DrawIText(&centertextfmt, strbuf, &measureRect, true);
			c_portSize[1] = MAX(c_portSize[1], measureRect.H());
			c_portSize[0] = MAX(c_portSize[0], measureRect.W()+2);

            portRect = getPortRect({UnitPortVector::Input, i});
            pGraphics->DrawRect(&input_port_color, &portRect);
			pGraphics->DrawIText(&centertextfmt, strbuf, &portRect);
        }
        // Draw output ports
        for (int i=0;i<nOutputPorts;i++){
			snprintf(strbuf, 256, "%s", m_voiceManager->getUnit(m_unitId).getOutputName(i).c_str());
			pGraphics->DrawIText(&centertextfmt, strbuf, &measureRect, true);
			c_portSize[1] = MAX(c_portSize[1], measureRect.H());
			c_portSize[0] = MAX(c_portSize[0], measureRect.W()+2);

            portRect = getPortRect({UnitPortVector::Output, i});
            pGraphics->DrawRect(&output_port_color, &portRect);
			pGraphics->DrawIText(&centertextfmt, strbuf, &portRect);
        }


		// Draw user controls
		for (int i = 0; i < nParams; i++) {
			m_paramControls[i].Draw(pGraphics);
			_updateMinSize({ m_paramControls[i].getMinSize(),10 });
		}

		// Resize if too small
		resize(m_size);
		return true;
	}

	NDPoint<2, int> UnitControl::getPos() const {
		return NDPoint<2, int>(m_x, m_y);
	}

	NDPoint<2, int> UnitControl::getSize() const {
		return m_size;
	}

	NDPoint<2, int> UnitControl::getPortPos(UnitPortVector a_portVector) const {
		int portId = a_portVector.id;
        int nPorts;
        int x = 0, y = 0;
        switch(a_portVector.type){
            case UnitPortVector::Input:
                nPorts = m_voiceManager->getUnit(m_unitId).getNumInputs();
                x = m_x + c_portSize[0]/2 + 1;
                y = m_y + mRECT.H() /nPorts*portId + c_portSize[1]/2;
				y += (mRECT.H() - mRECT.H() / (nPorts + 1)*nPorts) / nPorts; // center ports vertically
                break;
            case UnitPortVector::Output:
                nPorts = m_voiceManager->getUnit(m_unitId).getNumOutputs();
                x = m_x + mRECT.W() - c_portSize[0]/2 - 1;
                y = m_y + mRECT.H() /nPorts*portId + c_portSize[1]/2;
				y += (mRECT.H() - mRECT.H() / (nPorts+1)*nPorts) / nPorts; // center ports vertically
                break;
            case UnitPortVector::Null:
            default:
                break;
        }
        return NDPoint<2,int>(x,y);
	}

    IRECT UnitControl::getPortRect(UnitPortVector a_portVector) const {
        NDPoint<2,int> portPos = getPortPos(a_portVector);
        return IRECT{portPos[0]-c_portSize[0]/2,portPos[1]-c_portSize[1]/2,portPos[0]+c_portSize[0]/2,portPos[1]+c_portSize[1]/2};
    }

    UnitPortVector UnitControl::getSelectedPort(int x, int y) const {
        int nInputPorts = m_voiceManager->getUnit(m_unitId).getNumInputs();
        int nOutputPorts = m_voiceManager->getUnit(m_unitId).getNumOutputs();
        IRECT portRect;
		for (int i = 0; i < nInputPorts; i++) {
            portRect = getPortRect({UnitPortVector::Input, i});
            if(portRect.Contains(x,y)){
                return {UnitPortVector::Input, i};
            }
		}
        for (int i = 0; i < nOutputPorts; i++) {
            portRect = getPortRect({UnitPortVector::Output, i});
            if(portRect.Contains(x,y)){
                return {UnitPortVector::Output, i};
            }
        }
		return {UnitPortVector::Null, 0};
	}

	int UnitControl::getSelectedParam(int x, int y) {
		int selectedParam = -1;
		for (int i = 0; i < m_paramControls.size(); i++) {
			if (m_paramControls[i].GetRECT()->Contains(x, y)) {
				selectedParam = i;
			}
		}
		return selectedParam;
	}

	void UnitControl::setUnitId(int a_newUnitId)
	{
		m_unitId = a_newUnitId; 
		int nParams = m_voiceManager->getUnit(m_unitId).getNumParameters();
		m_paramControls.clear();
		for (int i = 0; i < nParams; i++) {
			m_paramControls.push_back(ITextSlider(mPlug, m_voiceManager, m_unitId, i, IRECT{ 0, 0, 0, 0 }));
		};
		resize(10);
	}
	int UnitControl::getUnitId() const
	{
		return m_unitId;
	}
}

