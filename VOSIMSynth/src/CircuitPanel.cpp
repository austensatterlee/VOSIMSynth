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

#include "CircuitPanel.h"
#include "UI.h"
#include <DSPMath.h>
#include <utility>

namespace syn
{
	void CircuitPanel::_deleteUnit(int unitctrlid) {
		WDL_MutexLock lock(&mPlug->mMutex);

		// Delete unit from instrument
		m_voiceManager->queueAction(DeleteUnit, { unitctrlid });

		// Update selection
		for (int i = 0; i < m_selectedUnits.size(); i++) {
			int selectedId = m_selectedUnits[i];
			if (selectedId == unitctrlid) {
				m_selectedUnits.erase(m_selectedUnits.begin() + i);
				i--;
			}
			else if (selectedId > unitctrlid) {
				m_selectedUnits[i]--;
			}
		}

		// Delete unit controller
		m_unitControls.erase(m_unitControls.begin() + unitctrlid);
		for (int i = 0; i < m_unitControls.size(); i++) {
			if (i >= unitctrlid) {
				m_unitControls[i]->setUnitId(i);
			}
		}

		m_lastClickedUnit = -1;
		m_lastClickedUnitPort = { UnitPortVector::Null, 0 };
	}

	void CircuitPanel::_deleteWire(ConnectionRecord a_conn) const {
		WDL_MutexLock lock(&mPlug->mMutex);
		switch (a_conn.type) {
		case ConnectionRecord::Internal:
			m_voiceManager->queueAction(DisconnectInternal,
			{ a_conn.from_id, a_conn.from_port, a_conn.to_id, a_conn.to_port });
			break;
		case ConnectionRecord::Input:
			m_voiceManager->queueAction(DisconnectInput,
			{ a_conn.from_port, a_conn.to_id, a_conn.to_port });
			break;
		case ConnectionRecord::Output:
			m_voiceManager->queueAction(DisconnectOutput,
			{ a_conn.from_port, a_conn.to_id, a_conn.to_port });
			break;
		case ConnectionRecord::Null:
		default:
			break;
		}
	}

	void CircuitPanel::OnMouseOver(int x, int y, IMouseMod* pMod) {
		m_lastSelectedUnit = getSelectedUnit(x, y);
		if (m_lastSelectedUnit >= 0) { // propogate to unit
			m_unitControls[m_lastSelectedUnit]->onMouseOver(x, y, pMod);
		}
		else { // detect nearest wire
			m_lastMousePos = NDPoint<2, int>(x, y);
			m_nearestWire = _getNearestWire(m_lastMousePos[0], m_lastMousePos[1]);
		}
		m_lastMousePos = NDPoint<2, int>{ x,y };
		m_lastMouseState = *pMod;
	}

	void CircuitPanel::OnMouseDown(int x, int y, IMouseMod* pMod) {
		m_lastClickedUnit = getSelectedUnit(x, y);
		m_lastClickedCircuitPort = getSelectedPort(x, y);

		if (!pMod->S && !isUnitInSelection(m_lastClickedUnit)) { // clear selection if clicking on a unit outside selection
			clearUnitSelection();
		}

		if (pMod->L) {
			if (m_lastClickedUnit >= 0) { // clicking on a unit
				shared_ptr<UnitControlContainer> unitCtrl = m_unitControls[m_lastClickedUnit];
				// propogate event to unit
				if (!unitCtrl->onMouseDown(x, y, pMod)) { // if the only the container was hit
					if (pMod->S) {
						if (!isUnitInSelection(m_lastClickedUnit)) { // add unit to selection when shift is pressed
							m_selectedUnits.push_back(m_lastClickedUnit);
							m_unitControls[m_lastClickedUnit]->setIsSelected(true);
						}
						else { // remove unit already in selection when shift is pressed
							removeUnitFromSelection(m_lastClickedUnit);
						}
					}
				}

				m_lastClickedUnitPort = unitCtrl->getSelectedPort(x, y);

				if (m_lastClickedUnitPort.type != UnitPortVector::Null) {
					m_currAction = CONNECTING;
				}
				else {
					m_currAction = MODIFYING_UNIT;
				}
			}
			else if (m_lastClickedCircuitPort.type != CircuitPortVector::Null) { // clicking on a circuit port
				m_currAction = CONNECTING;
				clearUnitSelection();
			}
			else { // clicking on open space
				m_currAction = SELECTING;
			}
		}
		m_lastMousePos = NDPoint<2, int>{ x,y };
		m_lastMouseState = *pMod;
		m_lastClickPos = m_lastMousePos;
		m_lastClickState = *pMod;
	}

	void CircuitPanel::OnMouseUp(int x, int y, IMouseMod* pMod) {
		int currSelectedUnit = getSelectedUnit(x, y);
		CircuitPortVector currSelectedCircuitPort = getSelectedPort(x, y);
		UnitPortVector currSelectedUnitPort = { UnitPortVector::Null,0 };
		if (currSelectedUnit >= 0) {
			currSelectedUnitPort = m_unitControls[currSelectedUnit]->getSelectedPort(x, y);
		}

		if (m_currAction == CONNECTING) {
			ActionArgs args;
			if (currSelectedCircuitPort.type && currSelectedCircuitPort.type == m_lastClickedUnitPort.type) { // connect unit port to circuit port
				args.id1 = currSelectedCircuitPort.id;
				args.id2 = m_lastClickedUnit;
				args.id3 = m_lastClickedUnitPort.id;
				if (m_lastClickedUnitPort.type == UnitPortVector::Input) {
					m_voiceManager->queueAction(ConnectInput, args);
				}
				else {
					m_voiceManager->queueAction(ConnectOutput, args);
				}
			}
			else if (m_lastClickedCircuitPort.type && m_lastClickedCircuitPort.type == currSelectedUnitPort.type) { // connect circuit port to unit port
				args.id1 = m_lastClickedCircuitPort.id;
				args.id2 = currSelectedUnit;
				args.id3 = currSelectedUnitPort.id;
				if (currSelectedUnitPort.type == UnitPortVector::Input) {
					m_voiceManager->queueAction(ConnectInput, args);
				}
				else {
					m_voiceManager->queueAction(ConnectOutput, args);
				}
			}
			else if (m_lastClickedUnit != currSelectedUnit &&
				m_lastClickedUnitPort.type && currSelectedUnitPort.type &&
				m_lastClickedUnitPort.type != currSelectedUnitPort.type) {
				if (m_lastClickedUnitPort.type == UnitPortVector::Input) { //connect unit input to unit output
					args.id1 = currSelectedUnit;
					args.id2 = currSelectedUnitPort.id;
					args.id3 = m_lastClickedUnit;
					args.id4 = m_lastClickedUnitPort.id;
					m_voiceManager->queueAction(ConnectInternal, args);
				}
				else { //connect unit output to unit input
					args.id1 = m_lastClickedUnit;
					args.id2 = m_lastClickedUnitPort.id;
					args.id3 = currSelectedUnit;
					args.id4 = currSelectedUnitPort.id;
					m_voiceManager->queueAction(ConnectInternal, args);
				}
			}
		}
		else if (m_currAction == SELECTING) {
			IRECT selection = makeIRectFromPoints(m_lastClickPos[0], m_lastClickPos[1], x, y);
			m_selectedUnits = getUnitSelection(selection);
			for (int unitId : m_selectedUnits) {
				m_unitControls[unitId]->setIsSelected(true);
			}
		}
		else if (m_lastClickState.R) { // Right click
			if (_checkNearestWire()) { // Right clicking on a wire
				// Open wire context menu if distance to nearest wire is below threshold
				shared_ptr<IPopupMenu> wire_menu = make_shared<IPopupMenu>();
				wire_menu->AddItem("Delete", 0);
				IPopupMenu* selectedMenu = mPlug->GetGUI()->CreateIPopupMenu(wire_menu.get(), x, y);
				if (selectedMenu == wire_menu.get()) {
					switch (wire_menu->GetChosenItemIdx()) {
					case 0:
						_deleteWire(m_nearestWire.record);
						break;
					default:
						break;
					}
				}
			}
			else if (currSelectedUnit >= 0) { // Right clicking on a unit
				IPopupMenu unitmenu;
				unitmenu.AddItem("Delete");
				IPopupMenu* selectedmenu = mPlug->GetGUI()->CreateIPopupMenu(&unitmenu, x, y);
				if (selectedmenu == &unitmenu) {
					int selectedItem = selectedmenu->GetChosenItemIdx();
					if (selectedItem == 0) { // delete
						if (isUnitInSelection(currSelectedUnit)) {
							while (m_selectedUnits.size() > 0) {
								_deleteUnit(m_selectedUnits.back());
							}
						}
						else {
							_deleteUnit(currSelectedUnit);
						}
					}
				}
			}
			else if (currSelectedUnit == -1) { // Right clicking on open space
			 // Open unit builder context menu
				shared_ptr<IPopupMenu> main_menu = make_shared<IPopupMenu>();
				vector<shared_ptr<IPopupMenu>> sub_menus;
				_generateUnitFactoryMenu(main_menu, sub_menus);
				IPopupMenu* selectedMenu = mPlug->GetGUI()->CreateIPopupMenu(main_menu.get(), x, y);
				if (selectedMenu) {
					string unit_name = selectedMenu->GetItemText(selectedMenu->GetChosenItemIdx());
					_createUnit(unit_name, NDPoint<2,int>{ x, y });
				}
			}
		}
		else if (currSelectedUnit >= 0) { // propogate event to unit if no other actions need to be taken
			m_unitControls[currSelectedUnit]->onMouseUp(x, y, pMod);
		}
		m_currAction = NONE;
		m_lastMousePos = NDPoint<2, int>{ x,y };
		m_lastMouseState = *pMod;
	}

	void CircuitPanel::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) {
		if (m_lastClickedUnit >= 0) {
			if (m_currAction == MODIFYING_UNIT) {
				if (!m_selectedUnits.empty()) {
					for (int unitId : m_selectedUnits) {
						shared_ptr<UnitControlContainer> unitCtrl = m_unitControls[unitId];
						unitCtrl->onMouseDrag(x, y, dX, dY, pMod);
					}
				}
				else {
					shared_ptr<UnitControlContainer> unitCtrl = m_unitControls[m_lastClickedUnit];
					unitCtrl->onMouseDrag(x, y, dX, dY, pMod);
				}
			}
		}
		m_lastMousePos = NDPoint<2, int>{ x,y };
		m_lastMouseState = *pMod;
	}

	void CircuitPanel::OnMouseDblClick(int x, int y, IMouseMod* pMod) {
		int currSelectedUnit = getSelectedUnit(x, y);
		if (currSelectedUnit >= 0) {
			shared_ptr<UnitControlContainer> unitctrl = m_unitControls[currSelectedUnit];
			unitctrl->onMouseDblClick(x, y, pMod);
		}
		m_lastClickPos = NDPoint<2, int>{ x, y };
		m_lastClickState = *pMod;
	}

	void CircuitPanel::OnMouseWheel(int x, int y, IMouseMod* pMod, int d) {
		int currSelectedUnit = getSelectedUnit(x, y);
		if (currSelectedUnit >= 0) {
			shared_ptr<UnitControlContainer> unitctrl = m_unitControls[currSelectedUnit];
			unitctrl->onMouseWheel(x, y, pMod, d);
		}
	}

	bool CircuitPanel::Draw(IGraphics* pGraphics) {
		// Local palette
		IColor bg_color = { 255, 50, 50, 50 };
		IColor in_port_color = { 255, 100, 75, 75 };
		IColor out_port_color = { 255, 75, 75, 100 };
		IText textfmt{ 12, &COLOR_BLACK, "Helvetica", IText::kStyleNormal, IText::kAlignNear, 0, IText::kQualityClearType };
		pGraphics->FillIRect(&bg_color, &mRECT);

		// Handle continuous mouse click events
		if (m_lastMouseState.L || m_lastMouseState.R) {
			OnMouseDrag(m_lastMousePos[0], m_lastMousePos[1], 0, 0, &m_lastMouseState);
		}

		/* Debug frame */
		/*
		IRECT dbgrect = { 5,5,105,105 };
		pGraphics->FillIRect(&COLOR_WHITE, &dbgrect);
		char dbgtxt[256];
		sprintf(dbgtxt, "vm ticks: %d", m_voiceManager->getTickCount());
		pGraphics->DrawIText(&textfmt, dbgtxt, &dbgrect);
		dbgrect.T += 10;
		sprintf(dbgtxt, "app ticks: %d", ((VOSIMSynth*)mPlug)->getTickCount());
		pGraphics->DrawIText(&textfmt, dbgtxt, &dbgrect);
		dbgrect.T += 10;
		sprintf(dbgtxt, "voices: %d", m_voiceManager->getNumVoices());
		pGraphics->DrawIText(&textfmt, dbgtxt, &dbgrect);
		dbgrect.T += 10;
		sprintf(dbgtxt, "playing: %d", m_voiceManager->isPlaying());
		pGraphics->DrawIText(&textfmt, dbgtxt, &dbgrect);
		dbgrect.T += 10;
		sprintf(dbgtxt, "transport: %d", ((VOSIMSynth*)mPlug)->isTransportRunning());
		pGraphics->DrawIText(&textfmt, dbgtxt, &dbgrect);
		*/

		const Circuit& circ = m_voiceManager->getCircuit();
		int nUnits = circ.getNumUnits();
		// Draw unit controllers
		for (int i = 0; i < nUnits; i++) {
			if (i >= m_unitControls.size())
				continue;
			m_unitControls[i]->draw(pGraphics);
		}

		// Draw internal connections (connections between units)
		for (int toUnitId = 0; toUnitId < nUnits; toUnitId++) {
			int nPorts = circ.getUnit(toUnitId).getNumInputs();
			for (int toPortId = 0; toPortId < nPorts; toPortId++) {
				vector<pair<int, int>> connections = circ.getConnectionsToInternalInput(toUnitId, toPortId);
				UnitPortVector inPort = { UnitPortVector::Input, toPortId };
				NDPoint<2, int> pt1 = m_unitControls[toUnitId]->getPortPos(inPort);
				for (int j = 0; j < connections.size(); j++) {
					int fromUnitId = connections[j].first;
					int fromPortId = connections[j].second;
					if (fromUnitId >= m_unitControls.size())
						continue;
					UnitPortVector outPort = { UnitPortVector::Output, fromPortId };
					NDPoint<2, int> pt2 = m_unitControls[fromUnitId]->getPortPos(outPort);

					pGraphics->DrawLine(&COLOR_BLACK, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
					if (_checkNearestWire() && m_nearestWire.record == ConnectionRecord{ ConnectionRecord::Internal, fromUnitId,
						fromPortId, toUnitId, toPortId }) {
						pGraphics->DrawLine(&COLOR_GREEN, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
					}
				}
			}
		}

		// Draw circuit inputs
		int nInputPorts = circ.getNumInputs();
		IRECT m_portIRECT;
		for (int i = 0; i < nInputPorts; i++) {
			m_portIRECT = getPortRect({ CircuitPortVector::Input,i });
			pGraphics->FillIRect(&in_port_color, &m_portIRECT);
		}

		// Draw wires to circuit input
		for (int circPortId = 0; circPortId < nInputPorts; circPortId++) {
			NDPoint<2, int> pt1 = getPortPos({ CircuitPortVector::Input, circPortId });
			vector<pair<int, int>> connections = circ.getConnectionsToCircuitInput(circPortId);
			for (int j = 0; j < connections.size(); j++) {
				int toUnitId = connections[j].first;
				int toPortId = connections[j].second;
				if (toUnitId >= m_unitControls.size())
					continue;
				UnitPortVector toPort = { UnitPortVector::Input, toPortId };
				NDPoint<2, int> pt2 = m_unitControls[toUnitId]->getPortPos(toPort);
				pGraphics->DrawLine(&COLOR_BLACK, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
				if (_checkNearestWire() &&
					m_nearestWire.record == ConnectionRecord{ ConnectionRecord::Input, 0, circPortId, toUnitId, toPortId }) {
					pGraphics->DrawLine(&COLOR_GREEN, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
				}
			}
		}

		// Draw circuit outputs
		int nOutputPorts = circ.getNumOutputs();
		for (int i = 0; i < nOutputPorts; i++) {
			m_portIRECT = getPortRect({ CircuitPortVector::Output,i });
			pGraphics->FillIRect(&out_port_color, &m_portIRECT);
		}
		// Draw wires to circuit output
		for (int circPortId = 0; circPortId < nOutputPorts; circPortId++) {
			NDPoint<2, int> pt1 = getPortPos({ CircuitPortVector::Output, circPortId });
			vector<pair<int, int>> connections = circ.getConnectionsToCircuitOutput(circPortId);
			for (int j = 0; j < connections.size(); j++) {
				int fromUnitId = connections[j].first;
				int fromPortId = connections[j].second;
				if (fromUnitId >= m_unitControls.size())
					continue;
				UnitPortVector toPort = { UnitPortVector::Output, fromPortId };
				NDPoint<2, int> pt2 = m_unitControls[fromUnitId]->getPortPos(toPort);
				pGraphics->DrawLine(&COLOR_BLACK, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
				if (_checkNearestWire() &&
					m_nearestWire.record == ConnectionRecord{ ConnectionRecord::Output, 0, circPortId, fromUnitId, fromPortId }) {
					pGraphics->DrawLine(&COLOR_GREEN, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
				}
			}
		}

		if (m_currAction == CONNECTING) {
			pGraphics->DrawLine(&COLOR_GRAY, m_lastClickPos[0], m_lastClickPos[1], m_lastMousePos[0], m_lastMousePos[1],
				nullptr, true);
		}
		else if (m_currAction == SELECTING) {
			IRECT selection = makeIRectFromPoints(m_lastClickPos[0], m_lastClickPos[1], m_lastMousePos[0], m_lastMousePos[1]);
			pGraphics->DrawRect(&COLOR_GRAY, &selection);
		}
		SetDirty(false);
		return true;
	}

	int CircuitPanel::getSelectedUnit(int x, int y) const {
		int selectedUnit = -1;
		for (shared_ptr<UnitControlContainer> unitControl : m_unitControls) {
			if (unitControl->isHit(x, y)) {
				selectedUnit = unitControl->getUnitId();
			}
		}
		return selectedUnit;
	}

	vector<int> CircuitPanel::getUnitSelection(IRECT& a_rect) {
		vector<int> selectedUnits;
		for (shared_ptr<UnitControlContainer> unitControl : m_unitControls) {
			if (unitControl->isHit(a_rect)) {
				selectedUnits.push_back(unitControl->getUnitId());
			}
		}
		return selectedUnits;
	}

	void CircuitPanel::clearUnitSelection() {
		for (int unitId : m_selectedUnits) {
			m_unitControls[unitId]->setIsSelected(false);
		}
		m_selectedUnits.clear();
	}

	bool CircuitPanel::addUnitToSelection(int a_unitid) {
		if (!isUnitInSelection(a_unitid)) {
			m_unitControls[a_unitid]->setIsSelected(true);
			m_selectedUnits.push_back(a_unitid);
			return true;
		}
		return false;
	}

	bool CircuitPanel::removeUnitFromSelection(int a_unitId) {
		vector<int>::iterator idx = find(m_selectedUnits.begin(), m_selectedUnits.end(), a_unitId);
		if (idx != m_selectedUnits.end()) {
			m_unitControls[a_unitId]->setIsSelected(false);
			m_selectedUnits.erase(idx);
			return true;
		}
		return false;
	}

	bool CircuitPanel::isUnitInSelection(int a_unitId) {
		return find(m_selectedUnits.begin(), m_selectedUnits.end(), a_unitId) != m_selectedUnits.end();
	}

	CircuitPanel::SelectedWire CircuitPanel::_getNearestWire(int x, int y) const {
		const Circuit& circ = m_voiceManager->getCircuit();
		double min_dist = -1;
		double min_input_port_dist = -1;
		ConnectionRecord min_dist_connection = { ConnectionRecord::Null, 0, 0, 0, 0 };
		NDPoint<2, int> pt( x, y );

		// Check wires between internal units
		for (shared_ptr<UnitControlContainer> unitControl : m_unitControls) {
			int toUnitId = unitControl->getUnitId();
			int nPorts = circ.getUnit(toUnitId).getNumInputs();
			for (int toPortId = 0; toPortId < nPorts; toPortId++) {
				vector<pair<int, int>> connections = circ.getConnectionsToInternalInput(toUnitId, toPortId);
				UnitPortVector inPort = { UnitPortVector::Input, toPortId };
				NDPoint<2, int> pt1 = m_unitControls[toUnitId]->getPortPos(inPort);
				for (int j = 0; j < connections.size(); j++) {
					int fromUnitId = connections[j].first;
					if (fromUnitId >= m_unitControls.size())
						continue;
					int fromPortId = connections[j].second;
					UnitPortVector outPort = { UnitPortVector::Output, fromPortId };
					NDPoint<2, int> pt2 = m_unitControls[fromUnitId]->getPortPos(outPort);

					double dist = pointLineDistance(pt, pt1, pt2);
					if (dist < min_dist || min_dist == -1) {
						min_dist = dist;
						min_input_port_dist = closestPointOnLine(pt, pt1, pt2).distFrom(pt1) * (1.0 / pt1.distFrom(pt2));
						min_dist_connection = { ConnectionRecord::Internal, fromUnitId, fromPortId,
							toUnitId, toPortId };
					}
				}
			}
		}
		// Check wires to circuit input
		int nInputPorts = circ.getNumInputs();
		for (int circPortId = 0; circPortId < nInputPorts; circPortId++) {
			NDPoint<2, int> pt1 = getPortPos({ CircuitPortVector::Input, circPortId });
			vector<pair<int, int>> connections = circ.getConnectionsToCircuitInput(circPortId);
			for (int j = 0; j < connections.size(); j++) {
				int toUnitId = connections[j].first;
				int toPortId = connections[j].second;
				if (toUnitId >= m_unitControls.size())
					continue;
				UnitPortVector toPort = { UnitPortVector::Input, toPortId };
				NDPoint<2, int> pt2 = m_unitControls[toUnitId]->getPortPos(toPort);
				double dist = pointLineDistance(pt, pt1, pt2);
				if (dist < min_dist || min_dist == -1) {
					min_dist = dist;
					min_input_port_dist = closestPointOnLine(pt, pt1, pt2).distFrom(pt1) * (1.0 / pt1.distFrom(pt2));
					min_dist_connection = { ConnectionRecord::Input, 0, circPortId, toUnitId, toPortId };
				}
			}
		}
		// Check wires to circuit output
		int nOutputPorts = circ.getNumOutputs();
		for (int circPortId = 0; circPortId < nOutputPorts; circPortId++) {
			NDPoint<2, int> pt1 = getPortPos({ CircuitPortVector::Output, circPortId });
			vector<pair<int, int>> connections = circ.getConnectionsToCircuitOutput(circPortId);
			for (int j = 0; j < connections.size(); j++) {
				int fromUnitId = connections[j].first;
				int fromPortId = connections[j].second;
				if (fromUnitId >= m_unitControls.size())
					continue;
				UnitPortVector toPort = { UnitPortVector::Output, fromPortId };
				NDPoint<2, int> pt2 = m_unitControls[fromUnitId]->getPortPos(toPort);
				double dist = pointLineDistance(pt, pt1, pt2);
				if (dist < min_dist || min_dist == -1) {
					min_dist = dist;
					min_input_port_dist = closestPointOnLine(pt, pt1, pt2).distFrom(pt1) * (1.0 / pt1.distFrom(pt2));
					min_dist_connection = { ConnectionRecord::Output, 0, circPortId, fromUnitId, fromPortId };
				}
			}
		}
		return SelectedWire{ min_dist_connection, static_cast<int>(min_dist), min_input_port_dist };
	}

	ByteChunk CircuitPanel::serialize() const {
		ByteChunk serialized;
		int nUnits = m_unitControls.size();
		serialized.Put<int>(&nUnits);
		for (int i = 0; i < nUnits; i++) {
			ByteChunk unitChunk = _serializeUnitControl(i);
			serialized.PutChunk(&unitChunk);
		}

		const Circuit& circuit = m_voiceManager->getCircuit();
		const vector<ConnectionRecord>& connRecords = circuit.getConnectionRecords();
		int nRecords = connRecords.size();
		serialized.Put<int>(&nRecords);
		for (int i = 0; i < nRecords; i++) {
			const ConnectionRecord& conn = connRecords[i];
			serialized.Put(&conn.type);
			serialized.Put<int>(&conn.from_id);
			serialized.Put<int>(&conn.from_port);
			serialized.Put<int>(&conn.to_id);
			serialized.Put<int>(&conn.to_port);
		}
		return serialized;
	}

	int CircuitPanel::unserialize(ByteChunk* serialized, int startPos) {
		while (m_unitControls.size()) {
			_deleteUnit(0);
		}
		int chunkpos = startPos;
		int nUnits;
		chunkpos = serialized->Get<int>(&nUnits, chunkpos);
		for (int i = 0; i < nUnits; i++) {
			chunkpos = _unserializeUnitControl(serialized, chunkpos);
		}

		int nRecords;
		chunkpos = serialized->Get<int>(&nRecords, chunkpos);
		ConnectionRecord rec;
		for (int i = 0; i < nRecords; i++) {
			chunkpos = serialized->Get(&rec.type, chunkpos);
			chunkpos = serialized->Get<int>(&rec.from_id, chunkpos);
			chunkpos = serialized->Get<int>(&rec.from_port, chunkpos);
			chunkpos = serialized->Get<int>(&rec.to_id, chunkpos);
			chunkpos = serialized->Get<int>(&rec.to_port, chunkpos);
			switch (rec.type) {
			case ConnectionRecord::Internal:
				m_voiceManager->queueAction(ConnectInternal, { rec.from_id, rec.from_port, rec.to_id, rec.to_port });
				break;
			case ConnectionRecord::Input:
				m_voiceManager->queueAction(ConnectInput, { rec.from_port, rec.to_id, rec.to_port });
				break;
			case ConnectionRecord::Output:
				m_voiceManager->queueAction(ConnectOutput, { rec.from_port, rec.to_id, rec.to_port });
				break;
			case ConnectionRecord::Null:
			default:
				break;
			}
		}
		return chunkpos;
	}

	ByteChunk CircuitPanel::_serializeUnitControl(int ctrlidx) const {
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
		for (int i = 0; i < nparams; i++) {
			double paramValue = unit.getParameter(i).get<double>();
			int paramPrecision = unit.getParameter(i).getPrecision();
			serialized.Put<double>(&paramValue);
			serialized.Put<int>(&paramPrecision);
		}
		return serialized;
	}

	int CircuitPanel::_unserializeUnitControl(ByteChunk* chunk, int startPos) {
		int chunkpos = startPos;
		int x, y, w, h;
		chunkpos = chunk->Get<int>(&x, chunkpos);
		chunkpos = chunk->Get<int>(&y, chunkpos);
		chunkpos = chunk->Get<int>(&w, chunkpos);
		chunkpos = chunk->Get<int>(&h, chunkpos);

		unsigned unit_class_id;
		chunkpos = chunk->Get<unsigned>(&unit_class_id, chunkpos);
		int unitId = _createUnit(unit_class_id, NDPoint<2, int>{ x, y });
		bool success = unitId >= 0;
		if (success)
			m_unitControls[unitId]->resize(NDPoint<2, int>(w, h));

		unsigned nparams;
		chunkpos = chunk->Get<unsigned>(&nparams, chunkpos);
		ActionArgs args;
		for (int paramId = 0; paramId < nparams; paramId++) {
			double paramValue;
			int paramPrecision;
			chunkpos = chunk->Get<double>(&paramValue, chunkpos);
			chunkpos = chunk->Get<int>(&paramPrecision, chunkpos);
			args.id1 = unitId;
			args.id2 = paramId;
			args.value = paramValue;
			if (success)
				m_voiceManager->queueAction(ModifyParam, args);
			args.id3 = paramPrecision;
			if (success)
				m_voiceManager->queueAction(ModifyParamPrecision, args);
		}

		return chunkpos;
	}

	void CircuitPanel::_generateUnitFactoryMenu(shared_ptr<IPopupMenu> main_menu,
		vector<shared_ptr<IPopupMenu>>& sub_menus) const {
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

	bool CircuitPanel::_checkNearestWire() const {
		return m_lastSelectedUnit < 0 && m_nearestWire.distFromWire < WIRE_THRESH && m_nearestWire.record.type != ConnectionRecord::Null;
	}

	NDPoint<2, int> CircuitPanel::getPortPos(const CircuitPortVector& a_vec) const {
		const Circuit& circ = m_voiceManager->getCircuit();
		int nInputs = circ.getNumInputs();
		int nOutputs = circ.getNumOutputs();
		NDPoint<2, int> portPos;
		switch (a_vec.type) {
		case CircuitPortVector::Input:
			portPos[0] = c_portSize;
			portPos[1] = a_vec.id * mRECT.H() / nInputs + c_portSize / 2;
			portPos[1] += (mRECT.H() - mRECT.H() / nInputs * (nInputs - 1) - c_portSize / 2) / nInputs; // center ports vertically
			break;
		case CircuitPortVector::Output:
			portPos[0] = mRECT.W() - c_portSize;
			portPos[1] = a_vec.id * mRECT.H() / nOutputs + c_portSize / 2;
			portPos[1] += (mRECT.H() - mRECT.H() / nOutputs * (nOutputs - 1) - c_portSize / 2) / nOutputs; // center ports vertically
			break;
		case CircuitPortVector::Null:
		default:
			break;
		}
		return portPos;
	}

	IRECT CircuitPanel::getPortRect(const CircuitPortVector& a_vec) const {
		NDPoint<2, int> portPos = getPortPos(a_vec);
		return{ portPos[0] - c_portSize / 2, portPos[1] - c_portSize / 2, portPos[0] + c_portSize / 2,
			portPos[1] + c_portSize / 2 };
	}

	CircuitPortVector CircuitPanel::getSelectedPort(int x, int y) const {
		const Circuit& circ = m_voiceManager->getCircuit();
		int nInputPorts = circ.getNumInputs();
		int nOutputPorts = circ.getNumOutputs();

		IRECT portRect;
		for (int i = 0; i < nInputPorts; i++) {
			portRect = getPortRect({ CircuitPortVector::Input, i });
			if (portRect.Contains(x, y)) {
				return{ CircuitPortVector::Input, i };
			}
		}
		for (int i = 0; i < nOutputPorts; i++) {
			portRect = getPortRect({ CircuitPortVector::Output, i });
			if (portRect.Contains(x, y)) {
				return{ CircuitPortVector::Output, i };
			}
		}
		return{ CircuitPortVector::Null, 0 };
	}
}