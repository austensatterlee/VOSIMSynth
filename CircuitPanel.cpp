#include "CircuitPanel.h"
#include "UI.h"
#include "VOSIMSynth.h"
#include "mutex.h"

namespace syn
{
	void CircuitPanel::updateInstrument() const
	{
		IPlugBase::IMutexLock lock(mPlug);
		m_vm->setMaxVoices(m_vm->getMaxVoices(), m_vm->getProtoInstrument());
	}

	void CircuitPanel::deleteUnit(int unitctrlid)
	{
		// Delete unit from instrument
		Unit* unit = m_unitControls[unitctrlid]->getUnit();
		int unitid = unit->getParent().getUnitId(unit);
		Instrument* instr = m_vm->getProtoInstrument();
		instr->removeUnit(unitid);
		// Delete unit controller
		mPlug->GetGUI()->mMutex.Enter();
		UnitControl* unitctrl = m_unitControls[unitctrlid];
		m_unitControls.erase(unitctrlid);
		delete unitctrl;
		mPlug->GetGUI()->mMutex.Leave();
		updateInstrument();
	}

	void CircuitPanel::deleteWire(int unitctrlid, int connection_idx)
	{
		Instrument* instr = m_vm->getProtoInstrument();
		ConnectionMetadata connection = instr->getConnectionsTo(unitctrlid)[connection_idx];
		instr->removeConnection(connection);
		m_nearestWire = getNearestWire(m_lastMousePos[0], m_lastMousePos[1]);
		updateInstrument();
	}

	void CircuitPanel::setSink(int unitctrlid)
	{
		Instrument* instr = m_vm->getProtoInstrument();
		Unit* unit = m_unitControls[unitctrlid]->getUnit();
		instr->setSinkId(instr->getUnitId(unit));
		for (pair<int, UnitControl*> ctrlpair : m_unitControls) {
			if (ctrlpair.second->m_is_sink) {
				ctrlpair.second->m_is_sink = false;
			}
		}
		m_unitControls[unitctrlid]->m_is_sink = true;
		updateInstrument();
	}

	void CircuitPanel::OnMouseOver(int x, int y, IMouseMod* pMod)
	{
		m_lastMousePos = NDPoint<2, int>(x, y);
		m_nearestWire = getNearestWire(m_lastMousePos[0], m_lastMousePos[1]);
	}

	void CircuitPanel::OnMouseDown(int x, int y, IMouseMod* pMod)
	{
		if (pMod->L)
			m_isMouseDown = 1;
		else if (pMod->R)
			m_isMouseDown = 2;
		m_lastClickPos = m_lastMousePos;
		m_lastSelectedUnit = getSelectedUnit(x, y);
		if (m_lastSelectedUnit >= 0) {
			UnitControl* unitCtrl = m_unitControls[m_lastSelectedUnit];
			unitCtrl->OnMouseDown(x, y, pMod); // propogate event to unit

			m_lastSelectedPort = unitCtrl->getSelectedPort(x, y);
			m_lastSelectedParam = unitCtrl->getSelectedParam(x, y);

			if (pMod->L && m_lastSelectedParam >= 0) {
				m_currAction = MOD_PARAM;
			} else if (pMod->L && m_lastSelectedPort.paramid >= 0) {
				m_currAction = CONNECT;
			} else if (pMod->C && pMod->L) {
				m_currAction = RESIZE;
			} else if (pMod->L) {
				m_currAction = MOVE;
			}
		}
	}

	void CircuitPanel::createUnit(string proto_name, int x, int y)
	{
		Instrument* instr = m_vm->getProtoInstrument();
		Unit* unit = m_unitFactory->createUnit(proto_name);
		int uid;
		if (unit->getUnitType() == STD_UNIT)
			uid = instr->addUnit(unit);
		else if (unit->getUnitType() == SOURCE_UNIT)
			uid = instr->addSource(dynamic_cast<SourceUnit*>(unit));
		mPlug->GetGUI()->mMutex.Enter();
		m_unitControls[uid] = new UnitControl(mPlug, m_vm, unit, x, y);
		mPlug->GetGUI()->mMutex.Leave();
		updateInstrument();
	}

	void CircuitPanel::OnMouseUp(int x, int y, IMouseMod* pMod)
	{
		Instrument* instr = m_vm->getProtoInstrument();
		int currSelectedUnit = getSelectedUnit(x, y);
		if (m_isMouseDown == 2 && currSelectedUnit == -1) { // Right clicking on open space
			if (m_isMouseDown == 2 && checkNearestWire()) {
				// Open wire context menu if distance to nearest wire is below threshold
				shared_ptr<IPopupMenu> wire_menu = make_shared<IPopupMenu>();
				wire_menu->AddItem("Delete",0);
				IPopupMenu* selectedMenu = mPlug->GetGUI()->CreateIPopupMenu(wire_menu.get(), x, y);
				if (selectedMenu == wire_menu.get()) {
					switch(wire_menu->GetChosenItemIdx()) {
					case 0:
						deleteWire(m_nearestWire[0], m_nearestWire[1]);
						break;
					default:
						break;
					}
				}
			}
			else {
				// Open unit builder context menu
				shared_ptr<IPopupMenu> main_menu = make_shared<IPopupMenu>();
				vector<shared_ptr<IPopupMenu>> sub_menus;
				generateUnitFactoryMenu(main_menu, sub_menus);
				IPopupMenu* selectedMenu = mPlug->GetGUI()->CreateIPopupMenu(main_menu.get(), x, y);
				if (selectedMenu) {
					string unit_name = selectedMenu->GetItemText(selectedMenu->GetChosenItemIdx());
					createUnit(unit_name, x, y);
				}
			}
		} else if (m_isMouseDown == 2 && currSelectedUnit >= 0) { // Right clicking on a unit
			IPopupMenu unitmenu;
			unitmenu.AddItem("Set sink");
			unitmenu.AddItem("Delete");
			unitmenu.AddItem("Set oscilloscope source");
			if (instr->isSourceUnit(currSelectedUnit)) {
				unitmenu.AddSeparator();
				unitmenu.AddItem("Set oscilloscope trigger");
				unitmenu.AddItem("Set primary source");
			}
			Unit* unit = m_unitControls[currSelectedUnit]->getUnit();
			IPopupMenu* selectedmenu = mPlug->GetGUI()->CreateIPopupMenu(&unitmenu, x, y);
			if (selectedmenu == &unitmenu) {
				int selectedItem = selectedmenu->GetChosenItemIdx();
				if (selectedItem == 0) { // Set sink
					setSink(currSelectedUnit);
				} else if (selectedItem == 1) { // Delete unit
					deleteUnit(currSelectedUnit);
				} else if (selectedItem == 2) { // Set oscilloscope source
					VOSIMSynth* vs = static_cast<VOSIMSynth*>(mPlug);
					vs->m_Oscilloscope->connectInput(instr->getUnitId(unit));
				} else if (selectedItem == 4) { // Set oscilloscope trigger
					VOSIMSynth* vs = static_cast<VOSIMSynth*>(mPlug);
					vs->m_Oscilloscope->connectTrigger(instr->getUnitId(unit));
				} else if (selectedItem == 5) { // Set primary source
					instr->resetPrimarySource(instr->getUnitId(unit));
					updateInstrument();
				}
			}
		} else if (m_currAction == CONNECT && currSelectedUnit >= 0) {
			m_vm->getProtoInstrument()->addConnection({currSelectedUnit, m_lastSelectedUnit, m_lastSelectedPort.paramid, m_lastSelectedPort.modaction});
			updateInstrument();
		} else if (currSelectedUnit >= 0) { // propogate event to unit if no other actions need to be taken
			m_unitControls[currSelectedUnit]->OnMouseUp(x, y, pMod);
		}
		m_currAction = NONE;
		m_isMouseDown = 0;
	}

	void CircuitPanel::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
	{
		NDPoint<2, int> currMousePos = NDPoint<2, int>(x, y);
		if (m_lastSelectedUnit >= 0) {
			UnitControl* unitCtrl = m_unitControls[m_lastSelectedUnit];
			NDPoint<2, int> unitPos = unitCtrl->getPos();
			if (m_currAction == MOD_PARAM) {
				unitCtrl->m_portLabels[m_lastSelectedParam].OnMouseDrag(x, y, dX, dY, pMod);
			} else if (m_currAction == RESIZE) {
				unitCtrl->resize(dX + unitCtrl->m_size);
			} else if (m_currAction == MOVE) {
				NDPoint<2, int> newUnitPos = currMousePos - m_lastMousePos + unitPos;
				unitCtrl->move(newUnitPos[0], newUnitPos[1]);
			}
		}
		m_lastMousePos = currMousePos;
	}

	void CircuitPanel::OnMouseDblClick(int x, int y, IMouseMod* pMod)
	{
		int currSelectedUnit = getSelectedUnit(x, y);
		if (currSelectedUnit >= 0) {
			UnitControl* unitctrl = m_unitControls[currSelectedUnit];
			unitctrl->OnMouseDblClick(x, y, pMod);
		}
	}

	bool CircuitPanel::Draw(IGraphics* pGraphics)
	{
		WDL_MutexLock lock(&pGraphics->mMutex);
		// Local palette
		IColor bg_color = globalPalette[PRIMARY][0];
		IText textfmt{12,&COLOR_RED,"Helvetica",IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType};

		pGraphics->FillIRect(&bg_color, &mRECT);
		for (pair<int, UnitControl*> unitpair : m_unitControls) {
			unitpair.second->Draw(pGraphics);
		}
		Instrument* instr = m_vm->getProtoInstrument();
		for (pair<int, UnitControl*> unitpair : m_unitControls) {
			const vector<ConnectionMetadata>& connections = instr->getConnectionsTo(unitpair.first);
			/* Draw wires */
			for (int j = 0; j < connections.size(); j++) {
				NDPoint<2, int> pt1 = m_unitControls[connections[j].srcid]->getOutputPos();
				SelectedPort selectedPort{connections[j].portid,connections[j].action};
				NDPoint<2, int> pt2 = m_unitControls[connections[j].targetid]->getPortPos(selectedPort);
				pGraphics->DrawLine(&COLOR_BLACK, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
				if(checkNearestWire() && m_nearestWire[0] == unitpair.first && m_nearestWire[1] == j ) {
					pGraphics->DrawLine(&COLOR_GREEN, pt1[0], pt1[1], pt2[0], pt2[1], nullptr, true);
				}
				/*
				//Draw debugging info for closest wires
				NDPoint<2, int> closestPoint = closestPointOnLine({ m_lastMousePos[0],m_lastMousePos[1] }, pt1, pt2);
				pGraphics->DrawLine(&COLOR_GREEN, m_lastMousePos[0], m_lastMousePos[1], closestPoint[0], closestPoint[1], nullptr, true);
				char strbuf[256];

				IRECT text_rect{m_lastMousePos[0] + 10, m_lastMousePos[1], m_lastMousePos[0] + 10, m_lastMousePos[1]};
				snprintf(strbuf, 256, "%.2f", dist);
				pGraphics->DrawIText(&textfmt, strbuf, &text_rect);

				text_rect = {m_lastMousePos[0] + 10, m_lastMousePos[1] - 10, m_lastMousePos[0] + 10, m_lastMousePos[1]};
				snprintf(strbuf, 256, "[%d, %d]", m_lastMousePos[0], m_lastMousePos[1]);
				pGraphics->DrawIText(&textfmt, strbuf, &text_rect);

				text_rect = NDPointToIRECT(pt1);
				snprintf(strbuf, 256, "[%d, %d]", pt1[0], pt1[1]);
				pGraphics->DrawIText(&textfmt, strbuf, &text_rect);

				text_rect = NDPointToIRECT(pt2);
				snprintf(strbuf, 256, "[%d, %d]", pt2[0], pt2[1]);
				pGraphics->DrawIText(&textfmt, strbuf, &text_rect);
				*/
				
			}
		}
		if (m_currAction == CONNECT) {
			pGraphics->DrawLine(&COLOR_GRAY, m_lastClickPos[0], m_lastClickPos[1], m_lastMousePos[0], m_lastMousePos[1], nullptr, true);
		}
		return true;
	}

	int CircuitPanel::getSelectedUnit(int x, int y)
	{
		int selectedUnit = -1;
		for (pair<int, UnitControl*> unitpair : m_unitControls) {
			if (unitpair.second->IsHit(x, y)) {
				selectedUnit = unitpair.first;
			}
		}
		return selectedUnit;
	}

	array<int, 3> CircuitPanel::getNearestWire(int x, int y)
	{
		Instrument* instr = m_vm->getProtoInstrument();
		double min_dist = -1;
		array<int, 3> min_dist_index = {0,0,-1};
		NDPoint<2, int> pt = {x,y};

		for (pair<int, UnitControl*> unitpair : m_unitControls) {
			const vector<ConnectionMetadata>& connections = instr->getConnectionsTo(unitpair.first);
			for (int j = 0; j < connections.size(); j++) {
				NDPoint<2, int> pt1 = m_unitControls[connections[j].srcid]->getOutputPos();
				SelectedPort selectedPort{connections[j].portid,connections[j].action};
				NDPoint<2, int> pt2 = m_unitControls[connections[j].targetid]->getPortPos(selectedPort);

				double dist = pointLineDistance(pt, pt1, pt2);
				if (dist < min_dist || min_dist == -1) {
					min_dist = dist;
					min_dist_index = {unitpair.first,j,static_cast<int>(min_dist)};
				}
			}
		}
		return min_dist_index;
	}

	ByteChunk CircuitPanel::serialize() const
	{
		ByteChunk serialized;

		unsigned int numunits = m_unitControls.size();
		serialized.PutBytes(&numunits, sizeof(unsigned int));
		for (pair<int, UnitControl*> ctrlpair : m_unitControls) {
			ByteChunk unitctrl_chunk = serializeUnitControl(ctrlpair.first);
			serialized.PutChunk(&unitctrl_chunk);
		}

		Instrument* instr = m_vm->getProtoInstrument();
		for (pair<int, UnitControl*> ctrlpair : m_unitControls) {
			int unitid = instr->getUnitId(ctrlpair.second->m_unit);
			const vector<ConnectionMetadata> connections = instr->getConnectionsTo(unitid);

			unsigned int numconnections = connections.size();
			serialized.PutBytes(&numconnections, sizeof(unsigned int));
			for (int j = 0; j < connections.size(); j++) {
				serialized.Put<ConnectionMetadata>(&connections[j]);
			}
		}
		return serialized;
	}

	int CircuitPanel::unserialize(ByteChunk* serialized, int startPos)
	{
		int chunkpos = startPos;
		Instrument* instr = m_vm->getProtoInstrument();
		vector<int> unitctrlids;
		for (pair<int, UnitControl*> ctrlpair : m_unitControls) {
			unitctrlids.push_back(ctrlpair.first);
		}
		for (int i = 0; i < unitctrlids.size(); i++) {
			deleteUnit(unitctrlids[i]);
		}

		// Unserialize units
		unsigned int numunits;
		chunkpos = serialized->Get<unsigned int>(&numunits, chunkpos);
		for (int i = 0; i < numunits; i++) {
			chunkpos = unserializeUnitControl(serialized, chunkpos);
			if (!chunkpos) {
				return 0;
			}
		}

		// Unserialize connections
		for (int i = 0; i < numunits; i++) {
			unsigned int numConns;
			chunkpos = serialized->Get<unsigned int>(&numConns, chunkpos);
			for (int j = 0; j < numConns; j++) {
				ConnectionMetadata conn;
				chunkpos = serialized->Get<ConnectionMetadata>(&conn, chunkpos);
				instr->addConnection(conn);
			}
		}
		updateInstrument();
		return chunkpos;
	}

	ByteChunk CircuitPanel::serializeUnitControl(int ctrlidx) const
	{
		ByteChunk serialized;
		UnitControl* uctrl = m_unitControls.at(ctrlidx);
		Instrument* instr = m_vm->getProtoInstrument();
		unsigned int unitClassId = uctrl->m_unit->getClassIdentifier();
		int unitid = instr->getUnitId(uctrl->m_unit);
		bool isSource = instr->isSourceUnit(unitid);
		bool isPrimarySource = isSource ? instr->isPrimarySource(unitid) : false;

		// Write unique class identifier
		serialized.Put<unsigned int>(&unitClassId);
		// Write the ID given to the unit by its parent circuit so it can be placed in the same location
		serialized.Put<int>(&unitid);
		// Write whether or not this unit is a SourceUnit
		serialized.Put<bool>(&isSource);
		// Write whether or not this unit is a primary source within its circuit
		serialized.Put<bool>(&isPrimarySource);
		// Write whether or not this unit is the audio sink
		serialized.Put<bool>(&uctrl->m_is_sink);

		// Write the unit's parameter configuration
		serialized.Put<size_t>(&uctrl->m_nParams);
		for (int i = 0; i < uctrl->m_nParams; i++) {
			double paramval = uctrl->m_unit->getParam(i);
			// Write the parameter's name
			serialized.PutStr(uctrl->m_unit->getParam(i).getName().c_str());
			// Write the parameter's current value
			serialized.Put<double>(&paramval);
		}

		// Write info related to this unit's GUI window
		serialized.Put<int>(&uctrl->m_size);
		serialized.Put<int>(&uctrl->m_x);
		serialized.Put<int>(&uctrl->m_y);
		return serialized;
	}

	int CircuitPanel::unserializeUnitControl(ByteChunk* chunk, int startPos)
	{
		int chunkpos = startPos;
		unsigned int unitClassId;
		int unitid;
		bool isSource, isPrimarySource, isSink;
		unsigned int numparams;
		int x, y;
		int size;
		Unit* unit;
		chunkpos = chunk->Get<unsigned int>(&unitClassId, chunkpos);
		chunkpos = chunk->Get<int>(&unitid, chunkpos);
		chunkpos = chunk->Get<bool>(&isSource, chunkpos);
		chunkpos = chunk->Get<bool>(&isPrimarySource, chunkpos);
		chunkpos = chunk->Get<bool>(&isSink, chunkpos);
		if (m_unitFactory->hasClassId(unitClassId)) {
			unit = m_unitFactory->createUnit(unitClassId);
		} else {
			return 0;
		}

		/* Unserialize parameters */
		chunkpos = chunk->Get<unsigned int>(&numparams, chunkpos);
		for (int i = 0; i < numparams; i++) {
			double paramval;
			WDL_String paramname;
			chunkpos = chunk->GetStr(&paramname, chunkpos);
			chunkpos = chunk->Get<double>(&paramval, chunkpos);
			int paramid = unit->getParamId(paramname.Get());
			if (paramid == -1) {
				continue;
			}
			unit->modifyParameter(paramid, paramval, SET);
		}
		chunkpos = chunk->Get<int>(&size, chunkpos);
		chunkpos = chunk->Get<int>(&x, chunkpos);
		chunkpos = chunk->Get<int>(&y, chunkpos);

		Instrument* instr = m_vm->getProtoInstrument();
		int uid = isSource ? instr->addSource(dynamic_cast<SourceUnit*>(unit), unitid) : instr->addUnit(unit, unitid);
		mPlug->GetGUI()->mMutex.Enter();
		m_unitControls[uid] = new UnitControl(mPlug, m_vm, unit, x, y, size);
		mPlug->GetGUI()->mMutex.Leave();
		if (isSink)
			setSink(uid);
		if (isPrimarySource)
			instr->resetPrimarySource(uid);
		updateInstrument();
		return chunkpos;
	}
}

