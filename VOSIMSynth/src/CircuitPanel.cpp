#include "CircuitPanel.h"
#include <utility>
#include <DSPMath.h>

namespace syn {
    void CircuitPanel::_deleteUnit(int unitctrlid)
    {
		WDL_MutexLock lock(&mPlug->mMutex);

		// Delete unit from instrument		
		m_voiceManager->doAction(DeleteUnit, { unitctrlid });

        // Delete unit controller
		m_unitControls.erase(m_unitControls.begin()+unitctrlid);
		for (int i = 0; i < m_unitControls.size();i++) {
			if (i >= unitctrlid) {
				m_unitControls.at(i)->setUnitId(i);
			}
		}

        m_lastSelectedUnit = -1;
        m_lastSelectedUnitPort = {UnitPortVector::Null, 0};
    }

    void CircuitPanel::_deleteWire(ConnectionRecord a_conn) const {
		WDL_MutexLock lock(&mPlug->mMutex);
        switch (a_conn.type) {
            case ConnectionRecord::Internal:
                m_voiceManager->doAction(DisconnectInternal,
	                                            {a_conn.from_id, a_conn.from_port, a_conn.to_id, a_conn.to_port});
                break;
            case ConnectionRecord::Input:
                m_voiceManager->doAction(DisconnectInput,
	                                            {a_conn.from_port, a_conn.to_id, a_conn.to_port});
                break;
            case ConnectionRecord::Output:
                m_voiceManager->doAction(DisconnectOutput,
	                                            {a_conn.from_port, a_conn.to_id, a_conn.to_port});
                break;
            case ConnectionRecord::Null:
            default:
                break;
        }
    }

	void CircuitPanel::registerUnitControl(shared_ptr<Unit> a_unit, shared_ptr<UnitControl> a_unitControl) {
		m_unitControlMap[a_unit->getClassIdentifier()] = a_unitControl;
    }

	void CircuitPanel::OnMouseOver(int x, int y, IMouseMod* pMod)
    {
        m_lastMousePos = NDPoint<2, int>(x, y);
        m_nearestWire = getNearestWire(m_lastMousePos[0], m_lastMousePos[1]);
    }

    void CircuitPanel::OnMouseDown(int x, int y, IMouseMod* pMod)
    {
        m_lastSelectedUnit = getSelectedUnit(x, y);
        m_lastSelectedCircuitPort = getSelectedPort(x, y);
		if (pMod->L) {
			if (m_lastSelectedUnit >= 0) {
				shared_ptr<UnitControlContainer> unitCtrl = m_unitControls.at(m_lastSelectedUnit);
				unitCtrl->onMouseDown(x, y, pMod); // propogate event to unit

				m_lastSelectedUnitPort = unitCtrl->getSelectedPort(x, y);

				if (m_lastSelectedUnitPort.type != UnitPortVector::Null) {
					m_currAction = CONNECT;
				}
				else {
					m_currAction = MODIFY_UNIT;
				}
			}
			else if (m_lastSelectedCircuitPort.type != CircuitPortVector::Null) {
				m_currAction = CONNECT;
			}
		}
        m_lastClickPos = m_lastMousePos;
        m_lastClickState = *pMod;
        m_lastMousePos = m_lastClickPos;
        m_lastMouseState = *pMod;
    }

    int CircuitPanel::_createUnit(string proto_name, int x, int y)
    {
        int prototypeId = m_unitFactory->getPrototypeId(proto_name);
		return _createUnit(prototypeId, x, y);
    }

    void CircuitPanel::OnMouseUp(int x, int y, IMouseMod* pMod)
    {
        int currSelectedUnit = getSelectedUnit(x, y);
        CircuitPortVector currSelectedCircuitPort = getSelectedPort(x, y);
        UnitPortVector currSelectedUnitPort = {UnitPortVector::Null,0};
        if (currSelectedUnit >= 0) {
            currSelectedUnitPort = m_unitControls.at(currSelectedUnit)->getSelectedPort(x, y);
        }
        if (m_currAction == CONNECT) {
            MuxArgs args;
            if (currSelectedCircuitPort.type && currSelectedCircuitPort.type == m_lastSelectedUnitPort.type) {
                // connect unit port to circuit port
                args.id1 = currSelectedCircuitPort.id;
                args.id2 = m_lastSelectedUnit;
                args.id3 = m_lastSelectedUnitPort.id;
                if (m_lastSelectedUnitPort.type == UnitPortVector::Input) {
                    m_voiceManager->doAction(ConnectInput, args);
                } else {
                    m_voiceManager->doAction(ConnectOutput, args);
                }
            } else if (m_lastSelectedCircuitPort.type && m_lastSelectedCircuitPort.type == currSelectedUnitPort.type) {
                // connect circuit port to unit port
                args.id1 = m_lastSelectedCircuitPort.id;
                args.id2 = currSelectedUnit;
                args.id3 = currSelectedUnitPort.id;
                if (currSelectedUnitPort.type == UnitPortVector::Input) {
                    m_voiceManager->doAction(ConnectInput, args);
                } else {
                    m_voiceManager->doAction(ConnectOutput, args);
                }
            } else if (m_lastSelectedUnit != currSelectedUnit &&
                       m_lastSelectedUnitPort.type && currSelectedUnitPort.type &&
                       m_lastSelectedUnitPort.type != currSelectedUnitPort.type) {
                if (m_lastSelectedUnitPort.type == UnitPortVector::Input) {
                    //connect unit input to unit output
                    args.id1 = currSelectedUnit;
                    args.id2 = currSelectedUnitPort.id;
                    args.id3 = m_lastSelectedUnit;
                    args.id4 = m_lastSelectedUnitPort.id;
                    m_voiceManager->doAction(ConnectInternal, args);
                } else {
                    //connect unit output to unit input
                    args.id1 = m_lastSelectedUnit;
                    args.id2 = m_lastSelectedUnitPort.id;
                    args.id3 = currSelectedUnit;
                    args.id4 = currSelectedUnitPort.id;
                    m_voiceManager->doAction(ConnectInternal, args);
                }
            }
        }
        if (m_lastClickState.R) { // Right click  
			if (_checkNearestWire()) { // Right clicking on a wire
				// Open wire context menu if distance to nearest wire is below threshold
				shared_ptr<IPopupMenu> wire_menu = make_shared<IPopupMenu>();
				wire_menu->AddItem("Delete", 0);
				IPopupMenu* selectedMenu = mPlug->GetGUI()->CreateIPopupMenu(wire_menu.get(), x, y);
				if (selectedMenu == wire_menu.get()) {
					switch (wire_menu->GetChosenItemIdx()) {
					case 0:
						_deleteWire(m_nearestWire.first);
						break;
					default:
						break;
					}
				}
			} else if (currSelectedUnit >= 0) { // Right clicking on a unit
				IPopupMenu unitmenu;
				unitmenu.AddItem("Delete");
				IPopupMenu* selectedmenu = mPlug->GetGUI()->CreateIPopupMenu(&unitmenu, x, y);
				if (selectedmenu == &unitmenu) {
					int selectedItem = selectedmenu->GetChosenItemIdx();
					if (selectedItem == 0) { // delete
						_deleteUnit(currSelectedUnit);
					}
				}
			}else if(currSelectedUnit==-1) { // Right clicking on open space     
				// Open unit builder context menu
				shared_ptr<IPopupMenu> main_menu = make_shared<IPopupMenu>();
				vector<shared_ptr<IPopupMenu>> sub_menus;
				_generateUnitFactoryMenu(main_menu, sub_menus);
				IPopupMenu* selectedMenu = mPlug->GetGUI()->CreateIPopupMenu(main_menu.get(), x, y);
				if (selectedMenu) {
					string unit_name = selectedMenu->GetItemText(selectedMenu->GetChosenItemIdx());
					_createUnit(unit_name, x, y);
				}
			}
        } else if (currSelectedUnit >= 0) { // propogate event to unit if no other actions need to be taken
            m_unitControls.at(currSelectedUnit)->onMouseUp(x, y, pMod);
        }
        m_currAction = NONE;
    }

    void CircuitPanel::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
    {
		WDL_MutexLock lock(&mPlug->mMutex);
        NDPoint<2, int> currMousePos = NDPoint<2, int>(x, y);
        if (m_lastSelectedUnit >= 0) {
            shared_ptr<UnitControlContainer> unitCtrl = m_unitControls.at(m_lastSelectedUnit);
            if (m_currAction == MODIFY_UNIT) {
                unitCtrl->onMouseDrag(x, y, dX, dY, pMod);
            } 
        }
        m_lastMousePos = currMousePos;
        m_lastMouseState = *pMod;
    }

    void CircuitPanel::OnMouseDblClick(int x, int y, IMouseMod* pMod)
    {
		WDL_MutexLock lock(&mPlug->mMutex);
        int currSelectedUnit = getSelectedUnit(x, y);
        if (currSelectedUnit >= 0) {
            shared_ptr<UnitControlContainer> unitctrl = m_unitControls.at(currSelectedUnit);
            unitctrl->onMouseDblClick(x, y, pMod);
        }
        m_lastClickPos = {x, y};
        m_lastClickState = *pMod;
    }

    bool CircuitPanel::Draw(IGraphics* pGraphics)
    {
		WDL_MutexLock lock(&mPlug->mMutex);
        // Local palette
        IColor bg_color = {255, 50, 50, 50};
        IColor in_port_color = {255, 100, 75, 75};
        IColor out_port_color = {255, 75, 75, 100};
        IText textfmt{12, &COLOR_RED, "Helvetica", IText::kStyleNormal, IText::kAlignNear, 0, IText::kQualityClearType};


		const Circuit& circ = m_voiceManager->getCircuit();
		int nUnits = circ.getNumUnits();
		// Draw unit controllers
        pGraphics->FillIRect(&bg_color, &mRECT);
		for (int i = 0; i < nUnits;i++) {
			if (i >= m_unitControls.size())
				continue;
            m_unitControls.at(i)->draw(pGraphics);
        }

        // Draw internal connections (connections between units)
		for (int toUnitId = 0; toUnitId < nUnits; toUnitId++) {
            int nPorts = circ.getUnit(toUnitId).getNumInputs();
            for (int toPortId = 0 ; toPortId < nPorts ; toPortId++) {
                vector<pair<int, int> > connections = circ.getConnectionsToInternalInput(toUnitId, toPortId);
                UnitPortVector inPort = {UnitPortVector::Input, toPortId};
                NDPoint<2, int> pt1 = m_unitControls.at(toUnitId)->getPortPos(inPort);
                for (int j = 0 ; j < connections.size() ; j++) {
                    int fromUnitId = connections[j].first;
                    int fromPortId = connections[j].second;
					if (fromUnitId >= m_unitControls.size())
						continue;
                    UnitPortVector outPort = {UnitPortVector::Output, fromPortId};
                    NDPoint<2, int> pt2 = m_unitControls.at(fromUnitId)->getPortPos(outPort);

                    pGraphics->DrawLine(&COLOR_BLACK, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
                    if (_checkNearestWire() && m_nearestWire.first == ConnectionRecord{ConnectionRecord::Internal, fromUnitId,
                                                                       fromPortId, toUnitId, toPortId}) {
                        pGraphics->DrawLine(&COLOR_GREEN, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
                    }
                }
            }
        }

        // Draw circuit inputs
        int nInputPorts = circ.getNumInputs();
        IRECT m_portIRECT;
        for(int i=0;i<nInputPorts;i++){
            m_portIRECT = getPortRect({CircuitPortVector::Input,i});
            pGraphics->FillIRect(&in_port_color, &m_portIRECT);
        }

        // Draw wires to circuit input
        for (int circPortId = 0 ; circPortId < nInputPorts ; circPortId++) {
            NDPoint<2, int> pt1 = getPortPos({CircuitPortVector::Input, circPortId});
            vector<pair<int, int> > connections = circ.getConnectionsToCircuitInput(circPortId);
            for (int j = 0 ; j < connections.size() ; j++) {
                int toUnitId = connections[j].first;
                int toPortId = connections[j].second;
				if (toUnitId >= m_unitControls.size())
					continue;
                UnitPortVector toPort = {UnitPortVector::Input, toPortId};
                NDPoint<2, int> pt2 = m_unitControls.at(toUnitId)->getPortPos(toPort);
                pGraphics->DrawLine(&COLOR_BLACK, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
                if (_checkNearestWire() &&
                    m_nearestWire.first == ConnectionRecord{ConnectionRecord::Input, 0, circPortId, toUnitId, toPortId})
                {
                    pGraphics->DrawLine(&COLOR_GREEN, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
                }
            }
        }

        // Draw circuit outputs
        int nOutputPorts = circ.getNumOutputs();
        for(int i=0;i<nOutputPorts;i++){
            m_portIRECT = getPortRect({CircuitPortVector::Output,i});
            pGraphics->FillIRect(&out_port_color, &m_portIRECT);
        }
        // Draw wires to circuit output
        for (int circPortId = 0 ; circPortId < nOutputPorts ; circPortId++) {
            NDPoint<2, int> pt1 = getPortPos({CircuitPortVector::Output, circPortId});
            vector<pair<int, int> > connections = circ.getConnectionsToCircuitOutput(circPortId);
            for (int j = 0 ; j < connections.size() ; j++) {
                int fromUnitId = connections[j].first;
                int fromPortId = connections[j].second;
				if (fromUnitId >= m_unitControls.size())
					continue;
                UnitPortVector toPort = {UnitPortVector::Output, fromPortId};
                NDPoint<2, int> pt2 = m_unitControls.at(fromUnitId)->getPortPos(toPort);
                pGraphics->DrawLine(&COLOR_BLACK, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
                if (_checkNearestWire() &&
                    m_nearestWire.first == ConnectionRecord{ConnectionRecord::Output, 0, circPortId, fromUnitId, fromPortId})
                {
                    pGraphics->DrawLine(&COLOR_GREEN, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
                }
            }
        }

        if (m_currAction == CONNECT) {
            pGraphics->DrawLine(&COLOR_GRAY, m_lastClickPos[0], m_lastClickPos[1], m_lastMousePos[0], m_lastMousePos[1],
                                nullptr, true);
        }
		SetDirty(false);
        return true;
    }

    int CircuitPanel::getSelectedUnit(int x, int y) const
    {
        int selectedUnit = -1;
        for (shared_ptr<UnitControlContainer> unitControl : m_unitControls) {
            if (unitControl->isHit(x, y)) {
                selectedUnit = unitControl->getUnitId();
            }
        }
        return selectedUnit;
    }

    pair<ConnectionRecord, int> CircuitPanel::getNearestWire(int x, int y) const
    {
        const Circuit& circ = m_voiceManager->getCircuit();
        double min_dist = -1;
        ConnectionRecord min_dist_connection = {ConnectionRecord::Null, 0, 0, 0, 0};
        NDPoint<2, int> pt = {x, y};

        // Check wires between internal units
        for (shared_ptr<UnitControlContainer> unitControl : m_unitControls) {
            int toUnitId = unitControl->getUnitId();
            int nPorts = circ.getUnit(toUnitId).getNumInputs();
            for (int toPortId = 0 ; toPortId < nPorts ; toPortId++) {
                vector<pair<int, int> > connections = circ.getConnectionsToInternalInput(toUnitId, toPortId);
                UnitPortVector inPort = {UnitPortVector::Input, toPortId};
                NDPoint<2, int> pt1 = m_unitControls.at(toUnitId)->getPortPos(inPort);
                for (int j = 0 ; j < connections.size() ; j++) {
                    int fromUnitId = connections[j].first;
					if (fromUnitId >= m_unitControls.size())
						continue;
                    int fromPortId = connections[j].second;
                    UnitPortVector outPort = {UnitPortVector::Output, fromPortId};
                    NDPoint<2, int> pt2 = m_unitControls.at(fromUnitId)->getPortPos(outPort);

                    double dist = pointLineDistance(pt, pt1, pt2);
                    if (dist < min_dist || min_dist == -1) {
                        min_dist = dist;
                        min_dist_connection = {ConnectionRecord::Internal, fromUnitId, fromPortId,
                                               toUnitId, toPortId};
                    }
                }
            }
        }
        // Check wires to circuit input
        int nInputPorts = circ.getNumInputs();
        for (int circPortId = 0 ; circPortId < nInputPorts ; circPortId++) {
            NDPoint<2, int> pt1 = getPortPos({CircuitPortVector::Input, circPortId});
            vector<pair<int, int> > connections = circ.getConnectionsToCircuitInput(circPortId);
            for (int j = 0 ; j < connections.size() ; j++) {
                int toUnitId = connections[j].first;
                int toPortId = connections[j].second;
				if (toUnitId >= m_unitControls.size())
					continue;
                UnitPortVector toPort = {UnitPortVector::Input, toPortId};
                NDPoint<2, int> pt2 = m_unitControls.at(toUnitId)->getPortPos(toPort);
                double dist = pointLineDistance(pt, pt1, pt2);
                if (dist < min_dist || min_dist == -1) {
                    min_dist = dist;
                    min_dist_connection = {ConnectionRecord::Input, 0, circPortId, toUnitId, toPortId};
                }
            }
        }
        // Check wires to circuit output
        int nOutputPorts = circ.getNumOutputs();
        for (int circPortId = 0 ; circPortId < nOutputPorts ; circPortId++) {
            NDPoint<2, int> pt1 = getPortPos({CircuitPortVector::Output, circPortId});
            vector<pair<int, int> > connections = circ.getConnectionsToCircuitOutput(circPortId);
            for (int j = 0 ; j < connections.size() ; j++) {
                int fromUnitId = connections[j].first;
                int fromPortId = connections[j].second; 
				if (fromUnitId >= m_unitControls.size())
					continue;
                UnitPortVector toPort = {UnitPortVector::Output, fromPortId};
                NDPoint<2, int> pt2 = m_unitControls.at(fromUnitId)->getPortPos(toPort);
                double dist = pointLineDistance(pt, pt1, pt2);
                if (dist < min_dist || min_dist == -1) {
                    min_dist = dist;
                    min_dist_connection = {ConnectionRecord::Output, 0, circPortId, fromUnitId, fromPortId};
                }
            }
        }
        return make_pair(min_dist_connection, min_dist);
    }

    ByteChunk CircuitPanel::serialize() const
    {
        ByteChunk serialized;
		int nUnits = m_unitControls.size();
		serialized.Put<int>(&nUnits);
		for (int i = 0; i < nUnits;i++) {
			ByteChunk unitChunk = _serializeUnitControl(i);
			serialized.PutChunk(&unitChunk);
		}

		const Circuit& circuit = m_voiceManager->getCircuit();
		const vector<ConnectionRecord>& connRecords = circuit.getConnectionRecords();
		int nRecords = connRecords.size();
		serialized.Put<int>(&nRecords);
		for (int i = 0; i < nRecords;i++) {
			const ConnectionRecord& conn = connRecords.at(i);
			serialized.Put(&conn.type);
			serialized.Put<int>(&conn.from_id);
			serialized.Put<int>(&conn.from_port);
			serialized.Put<int>(&conn.to_id);
			serialized.Put<int>(&conn.to_port);
		}
        return serialized;
    }

    int CircuitPanel::unserialize(ByteChunk* serialized, int startPos)
    {
		while(m_unitControls.size()) {
			_deleteUnit(0);
		}
        int chunkpos = startPos;
		int nUnits;
		chunkpos = serialized->Get<int>(&nUnits,chunkpos);
		for (int i = 0; i < nUnits;i++) {
			chunkpos = _unserializeUnitControl(serialized, chunkpos);
		}

		int nRecords;
		chunkpos = serialized->Get<int>(&nRecords, chunkpos);
		ConnectionRecord rec;
		for (int i = 0; i < nRecords;i++) {
			chunkpos = serialized->Get(&rec.type, chunkpos);
			chunkpos = serialized->Get<int>(&rec.from_id, chunkpos);
			chunkpos = serialized->Get<int>(&rec.from_port, chunkpos);
			chunkpos = serialized->Get<int>(&rec.to_id, chunkpos);
			chunkpos = serialized->Get<int>(&rec.to_port, chunkpos);
			switch (rec.type) {
			case ConnectionRecord::Internal:
				m_voiceManager->doAction(ConnectInternal, { rec.from_id, rec.from_port, rec.to_id, rec.to_port });
				break;
			case ConnectionRecord::Input: 
				m_voiceManager->doAction(ConnectInput, {rec.from_port, rec.to_id, rec.to_port });
				break;
			case ConnectionRecord::Output:
				m_voiceManager->doAction(ConnectOutput, {rec.from_port, rec.to_id, rec.to_port });
				break;
			case ConnectionRecord::Null:
			default: 
				break;
			}
		}
        return chunkpos;
    }

    ByteChunk CircuitPanel::_serializeUnitControl(int ctrlidx) const
    {
        ByteChunk serialized;
		const Unit& unit = m_voiceManager->getUnit(ctrlidx);
		// Serialize position and size
		int x = m_unitControls[ctrlidx]->getPos()[0];
		int y = m_unitControls[ctrlidx]->getPos()[1];
		int w = m_unitControls[ctrlidx]->getSize()[0];
		int h = m_unitControls[ctrlidx]->getSize()[1];
		serialized.Put<int>(&x);
		serialized.Put<int>(&y);
		serialized.Put<int>(&w);
		serialized.Put<int>(&h);
		// Serialize type of unit
		unsigned unit_class_id = unit.getClassIdentifier();
		serialized.Put<unsigned>(&unit_class_id);
		unsigned nparams = unit.getNumParameters();
		// Serialize parameters
		serialized.Put<unsigned>(&nparams);
		for (int i = 0; i < nparams;i++) {
			double paramvalue = unit.getParameter(i).getNorm();
			serialized.Put<double>(&paramvalue);
		}
        return serialized;
    }

    int CircuitPanel::_unserializeUnitControl(ByteChunk* chunk, int startPos)
    {
        int chunkpos = startPos;
		int x, y, w, h;
		chunkpos = chunk->Get<int>(&x, chunkpos);
		chunkpos = chunk->Get<int>(&y, chunkpos);
		chunkpos = chunk->Get<int>(&w, chunkpos);
		chunkpos = chunk->Get<int>(&h, chunkpos);

		unsigned unit_class_id;
		chunkpos = chunk->Get<unsigned>(&unit_class_id,chunkpos);
		int unitId = _createUnit(unit_class_id, x, y);
		m_unitControls[unitId]->resize(NDPoint<2,int>( w,h ));

		unsigned nparams;
		chunkpos = chunk->Get<unsigned>(&nparams, chunkpos);
		MuxArgs args;
		for (int paramId = 0; paramId < nparams; paramId++) {
			double paramvalue;
			chunkpos = chunk->Get<double>(&paramvalue, chunkpos);
			args.id1 = unitId;
			args.id2 = paramId;
			args.value = paramvalue;
			m_voiceManager->doAction(ModifyParamNorm, args);
		}
		
        return chunkpos;
    }

    void CircuitPanel::_generateUnitFactoryMenu(shared_ptr<IPopupMenu> main_menu,
                                                vector<shared_ptr<IPopupMenu>>& sub_menus) const
    {
        const set<string>& group_names = m_unitFactory->getGroupNames();
        int i = 0;
        for (const string& group_name : group_names) {
            while (sub_menus.size() <= i)
                sub_menus.push_back(make_shared<IPopupMenu>());
            main_menu->AddItem(group_name.c_str(), sub_menus[i].get());
            const vector<string>& proto_names = m_unitFactory->getPrototypeNames(group_name);
            int j = 0;
            for (const string& proto_name : proto_names) {
                sub_menus[i]->AddItem(proto_name.c_str(), j);
                j++;
            }
            i++;
        }
    }

    bool CircuitPanel::_checkNearestWire() const
    {
        return m_nearestWire.second < WIRE_THRESH && m_nearestWire.first.type != ConnectionRecord::Null;
    }

    NDPoint<2, int> CircuitPanel::getPortPos(const CircuitPortVector& a_vec) const
    {
        const Circuit& circ = m_voiceManager->getCircuit();
        int nInputs = circ.getNumInputs();
        int nOutputs = circ.getNumOutputs();
        NDPoint<2, int> portPos;
        switch (a_vec.type) {
            case CircuitPortVector::Input:
                portPos[0] = c_portSize;
                portPos[1] = a_vec.id * mRECT.H() / nInputs + c_portSize/2;
                portPos[1] += (mRECT.H() - mRECT.H() / nInputs*(nInputs - 1) - c_portSize / 2) / nInputs; // center ports vertically
                break;
            case CircuitPortVector::Output:
                portPos[0] = mRECT.W() - c_portSize;
                portPos[1] = a_vec.id * mRECT.H() / nOutputs + c_portSize/2;
				portPos[1] += (mRECT.H() - mRECT.H() / nOutputs*(nOutputs - 1) - c_portSize / 2) / nOutputs; // center ports vertically
                break;
            case CircuitPortVector::Null:
            default:
                break;
        }
        return portPos;
    }

    IRECT CircuitPanel::getPortRect(const CircuitPortVector& a_vec) const
    {
        NDPoint<2, int> portPos = getPortPos(a_vec);
        return {portPos[0] - c_portSize / 2, portPos[1] - c_portSize / 2, portPos[0] + c_portSize / 2,
                portPos[1] + c_portSize / 2};
    }

    CircuitPortVector CircuitPanel::getSelectedPort(int x, int y)
    {
        const Circuit& circ = m_voiceManager->getCircuit();
        int nInputPorts = circ.getNumInputs();
        int nOutputPorts = circ.getNumOutputs();

        IRECT portRect;
        for (int i = 0 ; i < nInputPorts ; i++) {
            portRect = getPortRect({CircuitPortVector::Input, i});
            if (portRect.Contains(x, y)) {
                return {CircuitPortVector::Input, i};
            }
        }
        for (int i = 0 ; i < nOutputPorts ; i++) {
            portRect = getPortRect({CircuitPortVector::Output, i});
            if (portRect.Contains(x, y)) {
                return {CircuitPortVector::Output, i};
            }
        }
        return {CircuitPortVector::Null, 0};
    }
}

