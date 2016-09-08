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

#include "Circuit.h"
#include "DSPMath.h"
#include <unordered_set>
#include <functional>

#include "common.h"
CEREAL_REGISTER_TYPE(syn::Circuit);
CEREAL_REGISTER_TYPE(syn::OutputUnit);
CEREAL_REGISTER_TYPE(syn::InputUnit);

using std::vector;
using std::unordered_set;

namespace syn
{
	Circuit::Circuit(const string& a_name) :
		Unit(a_name)
	{
		m_procGraph.fill(nullptr);
		InputUnit* inputUnit = new InputUnit("inputs");
		OutputUnit* outputUnit = new OutputUnit("outputs");
		m_units.add("inputs", inputUnit);
		m_units.add("outputs", outputUnit);
		m_inputUnit = inputUnit;
		m_outputUnit = outputUnit;
		addExternalInput_("left in");
		addExternalInput_("right in");
		addExternalOutput_("left out");
		addExternalOutput_("right out");
	}

	Circuit::Circuit(const Circuit& a_other) :
		Circuit(a_other.name()) 
	{
		const int* unitIndices = a_other.m_units.getIndices();
		for (int i = 0; i < a_other.m_units.size(); i++) {
			Unit* unit = a_other.m_units[unitIndices[i]];
			if(unit!=a_other.m_inputUnit && unit!=a_other.m_outputUnit)
				addUnit(a_other.m_units[unitIndices[i]]->clone(), unitIndices[i]);
		}
		for (int i = 0; i < a_other.m_connectionRecords.size(); i++) {
			const ConnectionRecord& rec = a_other.m_connectionRecords[i];
			connectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
		}
		copyFrom_(a_other);
	}

	Circuit& Circuit::operator=(const Circuit& a_other) {
		if (this != &a_other) {
			// delete old units
			const int* unitIndices = m_units.getIndices();
			for (int i = 0; i<m_units.size();) {
				Unit* unit = m_units[unitIndices[i]];
				if (unit != m_inputUnit && unit != m_outputUnit)
					removeUnit(unit);
				else
					i++;
			}
			// copy new units
			const int* otherUnitIndices = a_other.m_units.getIndices();
			for (int i = 0; i < a_other.m_units.size(); i++) {
				Unit* unit = a_other.m_units[otherUnitIndices[i]];
				if (unit != a_other.m_inputUnit && unit != a_other.m_outputUnit)
					addUnit(a_other.m_units[otherUnitIndices[i]]->clone(), otherUnitIndices[i]);
			}
			for (int i = 0; i < a_other.m_connectionRecords.size(); i++) {
				const ConnectionRecord& rec = a_other.m_connectionRecords[i];
				connectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
			}
			copyFrom_(a_other);
		}
		return *this;
	}

	Circuit::~Circuit() {
		for(int i=0;i<m_units.size();i++) {
			delete m_units[m_units.getIndices()[i]];
		}
	}

	void Circuit::process_() {
		// tick units in processing graph
		for (Unit** unit = m_procGraph.data(); *unit!=nullptr; unit++ ) {
			(*unit)->tick();
		}

		/* Push internally connected output signals to circuit output ports */
		for (int i = 0; i < m_outputSignals.size(); i++) {
			setOutputChannel_(i, m_outputUnit->getOutputValue(i));
		}
	}

	int Circuit::addUnit(Unit* a_unit) {
		// increment name until there is no collision
		while(m_units.contains(a_unit->name())) {
			a_unit->_setName(incrementSuffix(a_unit->name()));
		}
		int retval = m_units.add(a_unit->name(), a_unit);
		if (retval < 0)
			return retval;
		a_unit->_setParent(this);
		a_unit->setFs(fs());
		a_unit->setTempo(tempo());
		a_unit->m_midiData = m_midiData;
		_recomputeGraph();
		return retval;
	}

	bool Circuit::addUnit(Unit* a_unit, int a_unitId) {
		// increment name until there is no collision
		while (m_units.contains(a_unit->name())) {
			a_unit->_setName(incrementSuffix(a_unit->name()));
		}
		bool retval = m_units.add(a_unit->name(), a_unitId, a_unit);
		if (!retval)
			return false;
		a_unit->_setParent(this);
		a_unit->setFs(fs());
		a_unit->setTempo(tempo());
		a_unit->m_midiData = m_midiData;
		_recomputeGraph();
		return true;
	}

	const vector<ConnectionRecord>& Circuit::getConnections() const {
		return m_connectionRecords;
	};

	void Circuit::_recomputeGraph()
	{
		list<int> sinks;
		// Find sinks
		const int* unitIndices = m_units.getIndices();
		for (int i = 0; i < m_units.size(); i++) {
			int index = unitIndices[i];
			Unit* unit = m_units[index];
			if (!unit->numOutputs()) {
				sinks.push_back(index);
			}
		}
		sinks.push_back(getOutputUnitId());

		// reset proc graph
		m_procGraph.fill(nullptr);

		Unit** procGraph_ptr = m_procGraph.data();

		unordered_set<int> permClosedSet;
		unordered_set<int> tempClosedSet;
		std::function<void(int)> visit = [&](int unitId)
		{
			Unit* node = m_units[unitId];
			if (tempClosedSet.count(unitId) || permClosedSet.count(unitId))
				return;
			tempClosedSet.insert(unitId);

			int nPorts = node->numInputs();
			for (int i = 0; i < nPorts; i++) {
				const vector<std::pair<int, int> >& conns = getConnectionsToInternalInput(unitId, i);
				if (!conns.empty()) {
					for (const std::pair<int, int>& conn : conns) {
						visit(conn.first);
					}
				}
			}
			permClosedSet.insert(unitId);
			tempClosedSet.erase(unitId);
			*(procGraph_ptr++) = m_units[unitId];
		};
		// DFS
		while (!sinks.empty()) {
			int sinkId = sinks.back();
			sinks.pop_back();
			visit(sinkId);
		}
	}

	bool Circuit::isActive() const {
		for (Unit* const* unit = m_procGraph.data(); *unit != nullptr; unit++) {
			if ((*unit)->isActive())
				return true;
		}
		return false;
	}

	void Circuit::onFsChange_() {
		const int* unitIndices = m_units.getIndices();
		for (int i = 0; i < m_units.size(); i++) {
			m_units[unitIndices[i]]->setFs(fs());
		}
	}

	void Circuit::onTempoChange_() {
		const int* unitIndices = m_units.getIndices();
		for (int i = 0; i < m_units.size(); i++) {
			m_units[unitIndices[i]]->setTempo(tempo());
		}
	}

	void Circuit::onNoteOn_() {
		const int* unitIndices = m_units.getIndices();
		for (int i = 0; i < m_units.size(); i++) {
			m_units[unitIndices[i]]->noteOn(note(), velocity());
		}
	}

	void Circuit::onNoteOff_() {
		const int* unitIndices = m_units.getIndices();
		for (int i = 0; i < m_units.size(); i++) {
			m_units[unitIndices[i]]->noteOff(note(), velocity());
		}
	}

	void Circuit::onMidiControlChange_(int a_cc, double a_value) {
		const int* unitIndices = m_units.getIndices();
		for (int i = 0; i < m_units.size(); i++) {
			m_units[unitIndices[i]]->onMidiControlChange_(a_cc, a_value);
		}
	}

	void Circuit::onInputConnection_(int a_inputPort)
	{
		m_inputUnit->connectInput(a_inputPort, &getInputValue(a_inputPort));
	}

	void Circuit::onInputDisconnection_(int a_inputPort)
	{
		m_inputUnit->disconnectInput(a_inputPort);
	}

	Unit& Circuit::getUnit(int a_unitId) {
		return *m_units[a_unitId];
	}

	const Unit& Circuit::getUnit(int a_unitId) const {
		return *m_units[a_unitId];
	}

	int Circuit::addExternalInput_(const string& a_name) {
		int inputid = addInput_(a_name);
		m_inputUnit->addInput_(a_name);
		m_inputUnit->addOutput_(a_name);
		return inputid;
	}

	int Circuit::addExternalOutput_(const string& a_name) {
		int outputid = addOutput_(a_name);
		m_outputUnit->addOutput_(a_name);
		m_outputUnit->addInput_(a_name);
		return outputid;
	}

	int Circuit::getInputUnitId() const {
		return getUnitId(m_inputUnit);
	}

	int Circuit::getOutputUnitId() const {
		return getUnitId(m_outputUnit);
	}

	int Circuit::getNumUnits() const {
		return static_cast<int>(m_units.size());
	}

	Unit* const* Circuit::getProcGraph() const
	{
		return m_procGraph.data();
	}

	void Circuit::notifyMidiControlChange(int a_cc, double a_value) {
		onMidiControlChange_(a_cc, a_value);
	}

	vector<std::pair<int, int>> Circuit::getConnectionsToInternalInput(int a_unitId, int a_portid) const {
		int unitId = m_units.find(a_unitId);
		vector<std::pair<int, int>> connectedPorts;
		for (int i = 0; i < m_connectionRecords.size(); i++) {
			const ConnectionRecord& conn = m_connectionRecords[i];
			if (conn.to_id == unitId && conn.to_port == a_portid) {
				connectedPorts.push_back(std::make_pair(conn.from_id, conn.from_port));
			}
		}
		return connectedPorts;
	}
}