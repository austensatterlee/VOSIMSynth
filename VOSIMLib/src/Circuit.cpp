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
        const int* unitIndices = a_other.m_units.indices();
        for (int i = 0; i < a_other.m_units.size(); i++)
        {
            Unit* unit = a_other.m_units[unitIndices[i]];
            if (unit != a_other.m_inputUnit && unit != a_other.m_outputUnit)
                addUnit(a_other.m_units[unitIndices[i]]->clone(), unitIndices[i]);
        }
        for (int i = 0; i < a_other.m_connectionRecords.size(); i++)
        {
            const ConnectionRecord& rec = a_other.m_connectionRecords[i];
            connectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
        }
        copyFrom_(a_other);
    }

    Circuit& Circuit::operator=(const Circuit& a_other)
    {
        if (this != &a_other)
        {
            // delete old units
            const int* unitIndices = m_units.indices();
            for (int i = 0; i < m_units.size();)
            {
                Unit* unit = m_units[unitIndices[i]];
                if (unit != m_inputUnit && unit != m_outputUnit)
                    removeUnit(unit);
                else
                    i++;
            }
            // copy new units
            const int* otherUnitIndices = a_other.m_units.indices();
            for (int i = 0; i < a_other.m_units.size(); i++)
            {
                Unit* unit = a_other.m_units[otherUnitIndices[i]];
                if (unit != a_other.m_inputUnit && unit != a_other.m_outputUnit)
                    addUnit(a_other.m_units[otherUnitIndices[i]]->clone(), otherUnitIndices[i]);
            }
            for (int i = 0; i < a_other.m_connectionRecords.size(); i++)
            {
                const ConnectionRecord& rec = a_other.m_connectionRecords[i];
                connectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
            }
            copyFrom_(a_other);
        }
        return *this;
    }

    Circuit::~Circuit() { for (int i = 0; i < m_units.size(); i++) { delete m_units[m_units.indices()[i]]; } }

    Circuit::operator json() const
    {
        json j = Unit::operator json();
        j["units"] = json();
        for (int i = 0; i < m_units.size(); i++)
        {
            int id = m_units.indices()[i];
            j["units"][std::to_string(id)] = (*m_units[id]).operator json();
        }
        j["connections"] = json();
        for (auto& cr : m_connectionRecords)
        {
            json cr_j;
            cr_j["from"] = {getUnit(cr.from_id).name(), cr.from_port};
            cr_j["to"] = {getUnit(cr.to_id).name(), cr.to_port};
            j["connections"].push_back(cr_j);
        }
        return j;
    }

    Unit* Circuit::load(const json& j)
    {
        json units = j["units"];
        int inputUnitIndex = m_units.find(m_inputUnit);
        int outputUnitIndex = m_units.find(m_outputUnit);
        /* Load units */
        for (json::iterator it = units.begin(); it != units.end(); ++it)
        {
            int id = stoi(it.key());
            if (id == inputUnitIndex || id == outputUnitIndex)
                continue;
            const json unitJson = j["units"][it.key()];
            addUnit(Unit::fromJSON(unitJson), id);
        }

        /* Load connection records */
        json connections = j["connections"];
        for (json::iterator it = connections.begin(); it != connections.end(); ++it)
        {
            const json& cr_j(*it);
            ConnectionRecord cr;
            cr.from_port = cr_j["from"][1];
            cr.to_port = cr_j["to"][1];

            // Extract unit IDs depending on how they were stored
            if (cr_j["from"][0].is_string())
                cr.from_id = getUnitId(cr_j["from"][0].get<string>());
            else
                cr.from_id = cr_j["from"][0];
            if (cr_j["to"][0].is_string())
                cr.to_id = getUnitId(cr_j["to"][0].get<string>());
            else
                cr.to_id = cr_j["to"][0];
            connectInternal(cr.from_id, cr.from_port, cr.to_id, cr.to_port);
        }
        return this;
    }

    void Circuit::process_()
    {
        // tick units in processing graph
        for (Unit** unit = m_procGraph.data(); *unit != nullptr; unit++) { (*unit)->tick(); }

        /* Push internally connected output signals to circuit output ports */
        for (int i = 0; i < m_outputSignals.size(); i++) { setOutputChannel_(i, m_outputUnit->readOutput(i)); }
    }

    int Circuit::addUnit(Unit* a_unit)
    {
        // increment name until there is no collision
        while (m_units.contains(a_unit->name())) { a_unit->_setName(incrementSuffix(a_unit->name())); }
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

    bool Circuit::addUnit(Unit* a_unit, int a_unitId)
    {
        // increment name until there is no collision
        while (m_units.contains(a_unit->name())) { a_unit->_setName(incrementSuffix(a_unit->name())); }
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

    const vector<ConnectionRecord>& Circuit::getConnections() const { return m_connectionRecords; };

    void Circuit::_recomputeGraph()
    {
        list<int> sinks;
        // Find sinks
        for (int i = 0; i < m_units.size(); i++)
        {
            Unit* unit = m_units.getByIndex(i);
            if (!unit->numOutputs()) { sinks.push_back(m_units.find(unit)); }
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
                    for (int i = 0; i < nPorts; i++)
                    {
                        const vector<std::pair<int, int>>& conns = getConnectionsToInternalInput(unitId, i);
                        if (!conns.empty()) { for (const std::pair<int, int>& conn : conns) { visit(conn.first); } }
                    }
                    permClosedSet.insert(unitId);
                    tempClosedSet.erase(unitId);
                    *(procGraph_ptr++) = m_units[unitId];
                };
        // DFS
        while (!sinks.empty())
        {
            int sinkId = sinks.back();
            sinks.pop_back();
            visit(sinkId);
        }
    }

    bool Circuit::isActive() const
    {
        for (Unit*const * unit = m_procGraph.data(); *unit != nullptr; unit++)
        {
            if ((*unit)->isActive())
                return true;
        }
        return false;
    }

    void Circuit::onFsChange_()
    {
        const int* unitIndices = m_units.indices();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->setFs(fs()); }
    }

    void Circuit::onTempoChange_()
    {
        const int* unitIndices = m_units.indices();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->setTempo(tempo()); }
    }

    void Circuit::onNoteOn_()
    {
        const int* unitIndices = m_units.indices();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->noteOn(note(), velocity()); }
    }

    void Circuit::onNoteOff_()
    {
        const int* unitIndices = m_units.indices();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->noteOff(note(), velocity()); }
    }

    void Circuit::onMidiControlChange_(int a_cc, double a_value)
    {
        const int* unitIndices = m_units.indices();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->onMidiControlChange_(a_cc, a_value); }
    }

    void Circuit::onInputConnection_(int a_inputPort) { m_inputUnit->connectInput(a_inputPort, &readInput(a_inputPort)); }

    void Circuit::onInputDisconnection_(int a_inputPort) { m_inputUnit->disconnectInput(a_inputPort); }

    Unit& Circuit::getUnit(int a_unitId) { return *m_units[a_unitId]; }

    const Unit& Circuit::getUnit(int a_unitId) const { return *m_units[a_unitId]; }

    int Circuit::addExternalInput_(const string& a_name)
    {
        int inputid = addInput_(a_name);
        m_inputUnit->addInput_(a_name);
        m_inputUnit->addOutput_(a_name);
        return inputid;
    }

    int Circuit::addExternalOutput_(const string& a_name)
    {
        int outputid = addOutput_(a_name);
        m_outputUnit->addOutput_(a_name);
        m_outputUnit->addInput_(a_name);
        return outputid;
    }

    int Circuit::getInputUnitId() const { return getUnitId(m_inputUnit); }

    int Circuit::getOutputUnitId() const { return getUnitId(m_outputUnit); }

    int Circuit::getNumUnits() const { return static_cast<int>(m_units.size()); }

    Unit*const * Circuit::getProcGraph() const { return m_procGraph.data(); }

    void Circuit::notifyMidiControlChange(int a_cc, double a_value) { onMidiControlChange_(a_cc, a_value); }

    vector<std::pair<int, int>> Circuit::getConnectionsToInternalInput(int a_unitId, int a_portid) const
    {
        int unitId = m_units.find(a_unitId);
        vector<std::pair<int, int>> connectedPorts;
        for (int i = 0; i < m_connectionRecords.size(); i++)
        {
            const ConnectionRecord& conn = m_connectionRecords[i];
            if (conn.to_id == unitId && conn.to_port == a_portid) { connectedPorts.push_back(std::make_pair(conn.from_id, conn.from_port)); }
        }
        return connectedPorts;
    }
}
