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
#include <unordered_set>
#include <functional>

using std::vector;
using std::unordered_set;

namespace syn
{
	Circuit::Circuit() :
		Circuit("") {}

	Circuit::Circuit(const string& a_name) :
		Unit(a_name),
		m_units(256) {
		shared_ptr<PassthroughUnit> inputUnit = make_shared<PassthroughUnit>("inputs");
		shared_ptr<PassthroughUnit> outputUnit = make_shared<PassthroughUnit>("outputs");
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
		Circuit(a_other.getName()) {
		vector<int> unitIds = a_other.m_units.getIds();
		for (int i = 0; i < unitIds.size(); i++) {
			addUnit(shared_ptr<Unit>(a_other.m_units[unitIds[i]]->clone()), unitIds[i]);
		}
		for (int i = 0; i < a_other.m_connectionRecords.size(); i++) {
			const ConnectionRecord& rec = a_other.m_connectionRecords[i];
			connectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
		}
	}

	void Circuit::process_() {
		// tick units in processing graph
		for (Unit* unit : m_procGraph) {
			unit->tick();
		}

		/* Push internally connected output signals to circuit output ports */
		for (int i = 0; i < m_outputSignals.size(); i++) {
			setOutputChannel_(i, m_outputUnit->getOutputValue(i));
		}
	}

	int Circuit::addUnit(shared_ptr<Unit> a_unit) {
		int retval = m_units.add(a_unit.get()->getName(), a_unit);
		if (retval < 0)
			return retval;
		a_unit->_setParent(this);
		a_unit->setFs(getFs());
		a_unit->setTempo(getTempo());
		a_unit->m_midiData = m_midiData;
		_recomputeGraph();
		return retval;
	}

	bool Circuit::addUnit(shared_ptr<Unit> a_unit, int a_unitId) {
		bool retval = m_units.add(a_unit.get()->getName(), a_unitId, a_unit);
		if (!retval)
			return false;
		a_unit->_setParent(this);
		a_unit->setFs(getFs());
		a_unit->setTempo(getTempo());
		a_unit->m_midiData = m_midiData;
		_recomputeGraph();
		return true;
	}

	const vector<ConnectionRecord>& Circuit::getConnections() const {
		return m_connectionRecords;
	};

	Unit* Circuit::_clone() const {
		return new Circuit(*this);
	}

	void Circuit::_recomputeGraph()
	{
		list<int> sinks;
		m_procGraph.clear();
		// Find sinks
		const vector<int>& unitIds = m_units.getIds();
		for (const int& id : unitIds) {
			shared_ptr<Unit> unit = m_units[id];
			if (!unit->getNumOutputs()) {
				sinks.push_back(id);
			}
		}
		sinks.push_back(getOutputUnitId());

		unordered_set<int> permClosedSet;
		unordered_set<int> tempClosedSet;
		std::function<void(int)> visit = [&](int unitId)
		{
			shared_ptr<Unit> node = m_units[unitId];
			if (tempClosedSet.count(unitId) || permClosedSet.count(unitId))
				return;
			tempClosedSet.insert(unitId);

			int nPorts = node->getNumInputs();
			for (int i = 0; i < nPorts; i++) {
				const vector<pair<int, int> >& conns = getConnectionsToInternalInput(unitId, i);
				if (!conns.empty()) {
					for (const pair<int, int>& conn : conns) {
						visit(conn.first);
					}
				}
			}
			permClosedSet.insert(unitId);
			tempClosedSet.erase(unitId);
			m_procGraph.push_back(m_units[unitId].get());
		};
		// DFS
		while (!sinks.empty()) {
			int sinkId = sinks.back();
			sinks.pop_back();
			visit(sinkId);
		}
	}

	bool Circuit::isActive() const {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
			if ((*it)->isActive())
				return true;
		}
		return false;
	}

	void Circuit::onFsChange_() {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
			(*it)->setFs(getFs());
		}
	}

	void Circuit::onTempoChange_() {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
			(*it)->setTempo(getTempo());
		}
	}

	void Circuit::onNoteOn_() {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
			(*it)->noteOn(getNote(), getVelocity());
		}
	}

	void Circuit::onNoteOff_() {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
			(*it)->noteOff(getNote(), getVelocity());
		}
	}

	void Circuit::onMidiControlChange_(int a_cc, double a_value) {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
			(*it)->onMidiControlChange_(a_cc, a_value);
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

	Unit& Circuit::getUnit_(int a_unitId) {
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

	const Unit& Circuit::getUnit(int a_unitId) const {
		return *m_units[a_unitId];
	}

	int Circuit::getInputUnitId() const {
		return getUnitId(m_inputUnit);
	}

	int Circuit::getOutputUnitId() const {
		return getUnitId(m_outputUnit);
	}

	int Circuit::getNumUnits() const {
		return m_units.size();
	}

	const list<Unit*>& Circuit::getProcGraph() const
	{
		return m_procGraph;
	}

	void Circuit::notifyMidiControlChange(int a_cc, double a_value) {
		onMidiControlChange_(a_cc, a_value);
	}

	vector<pair<int, int>> Circuit::getConnectionsToInternalInput(int a_unitId, int a_portid) const {
		int unitId = m_units.getItemId(a_unitId);
		vector<pair<int, int>> connectedPorts;
		for (int i = 0; i < m_connectionRecords.size(); i++) {
			const ConnectionRecord& conn = m_connectionRecords[i];
			if (conn.to_id == unitId && conn.to_port == a_portid) {
				connectedPorts.push_back(make_pair(conn.from_id, conn.from_port));
			}
		}
		return connectedPorts;
	}
}