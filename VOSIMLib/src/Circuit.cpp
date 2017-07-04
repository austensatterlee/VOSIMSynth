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
        Unit(a_name),
        m_voiceIndex(0.0)
    {
        m_procGraph.fill(nullptr);
        InputUnit* inputUnit = new InputUnit("inputs");
        OutputUnit* outputUnit = new OutputUnit("outputs");
        m_units.add(inputUnit);
        m_units.add(outputUnit);
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
        const int* unitIndices = a_other.m_units.ids();
        for (int i = 0; i < a_other.m_units.size(); i++)
        {
            const Unit* unit = a_other.m_units[unitIndices[i]];
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
            const int* unitIndices = m_units.ids();
            for (int i = 0; i < m_units.size();)
            {
                const Unit* unit = m_units[unitIndices[i]];
                if (unit != m_inputUnit && unit != m_outputUnit)
                    removeUnit(*unit);
                else
                    i++;
            }
            // copy new units
            const int* otherUnitIndices = a_other.m_units.ids();
            for (int i = 0; i < a_other.m_units.size(); i++)
            {
                const Unit* unit = a_other.m_units[otherUnitIndices[i]];
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

    Circuit::~Circuit() { for (int i = 0; i < m_units.size(); i++) { delete m_units.getByIndex(i); } }

    bool Circuit::disconnectInternal(int a_fromId, int a_fromOutputPort, int a_toId, int a_toInputPort) {
        Unit* toUnit = m_units[a_toId];
        Unit* fromUnit = m_units[a_fromId];
        assert(toUnit->inputSource(a_toInputPort) == &fromUnit->readOutput(a_fromOutputPort, 0));

        bool result = toUnit->disconnectInput(a_toInputPort);

        // Find and remove the associated connection record stored in this Circuit
        ConnectionRecord record{a_fromId, a_fromOutputPort, a_toId, a_toInputPort};
        for (int i = 0; i < m_connectionRecords.size(); i++) {
            if (m_connectionRecords[i] == record) {
                result = true;
                m_connectionRecords.erase(m_connectionRecords.begin() + i);
                _recomputeGraph();
                break;
            }
        }
        return result;
    }

    Circuit::operator json() const
    {
        json j = Unit::operator json();
        j["units"] = json();
        for (int i = 0; i < m_units.size(); i++)
        {
            int id = m_units.ids()[i];
            j["units"][std::to_string(id)] = (*m_units[id]).operator json();
        }
        j["connections"] = json();
        for (auto& cr : m_connectionRecords)
        {
            json cr_j;
            cr_j["from"] = {cr.from_id, cr.from_port};
            cr_j["to"] = {cr.to_id, cr.to_port};
            j["connections"].push_back(cr_j);
        }
        return j;
    }

    Unit* Circuit::load(const json& j)
    {
        json units = j["units"];
        Unit* inputUnit = m_inputUnit;
        Unit* outputUnit = m_outputUnit;
        int inputUnitIndex = m_units.getIdFromItem(inputUnit);
        int outputUnitIndex = m_units.getIdFromItem(outputUnit);
        /* Load units */
        for (json::iterator it = units.begin(); it != units.end(); ++it)
        {
            int id = stoi(it.key());
            if (id == inputUnitIndex || id == outputUnitIndex)
                continue;
            const json unitJson = j["units"][it.key()];
            bool success = addUnit(Unit::fromJSON(unitJson), id);
            assert(success);
        }

        /* Load connection records */
        json connections = j["connections"];
        for (json::iterator it = connections.begin(); it != connections.end(); ++it)
        {
            const json& cr_j(*it);
            ConnectionRecord cr;
            cr.from_port = cr_j["from"][1];
            cr.to_port = cr_j["to"][1];

            cr.from_id = cr_j["from"][0];
            cr.to_id = cr_j["to"][0];
            bool success = connectInternal(cr.from_id, cr.from_port, cr.to_id, cr.to_port);
            assert(success);
        }
        return this;
    }

    void Circuit::reset()
    {
        const int* unitIndices = m_units.ids();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->reset(); }
    }

    void Circuit::process_()
    {
        // tick units in processing graph
        for (Unit** unit = m_procGraph.data(); *unit != nullptr; unit++)
        {
            (*unit)->tick();
        }

        /* Push internally connected output signals to circuit output ports */
        for (int i = 0; i < m_outputPorts.size(); i++)
        {
            for (int j = 0; j < getBufferSize(); j++)
            {
                writeOutput_(i, j, m_outputUnit->readOutput(i, j));
            }
        }
    }

    int Circuit::addUnit(Unit* a_unit)
    {
        int id = m_units.getUnusedId();
        addUnit(a_unit, id);
        return id;
    }

    bool Circuit::addUnit(Unit* a_unit, int a_unitId)
    {
        bool retval = m_units.add(a_unitId, a_unit);
        if (!retval)
            return false;
        a_unit->_setParent(this);
        a_unit->setFs(fs());
        a_unit->setTempo(tempo());
        a_unit->setBufferSize(getBufferSize());
        a_unit->m_midiData = m_midiData;
        _recomputeGraph();
        return true;
    }

    bool Circuit::removeUnit(const Unit& a_unit) {
        const Unit* unitPtr = &a_unit;
        int a_id = m_units.getIdFromItem(unitPtr);
        return removeUnit(a_id);
    }

    bool Circuit::removeUnit(int a_id) {
        if (a_id < 0)
            return false;
        Unit* unit = m_units[a_id];
        // Don't allow deletion of input or output unit
        if (unit == m_inputUnit || unit == m_outputUnit)
            return false;
        // Erase connections
        vector<ConnectionRecord> garbageList;
        const size_t nRecords = m_connectionRecords.size();
        for (int i = 0; i < nRecords; i++) {
            const ConnectionRecord& rec = m_connectionRecords[i];
            if (a_id == rec.to_id || a_id == rec.from_id) { garbageList.push_back(rec); }
        }
        for (int i = 0; i < garbageList.size(); i++) {
            const ConnectionRecord& rec = garbageList[i];
            disconnectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
        }
        m_units.removeById(a_id);
        delete unit;
        _recomputeGraph();
        return true;
    }

    bool Circuit::connectInternal(int a_fromId, int a_fromOutputPort, int a_toId, int a_toInputPort) {
        Unit* fromUnit = m_units[a_fromId];
        Unit* toUnit = m_units[a_toId];

        if (!fromUnit->hasOutput(a_fromOutputPort))
            return false;
        if (!toUnit->hasInput(a_toInputPort))
            return false;

        // remove record of old connection
        if (toUnit->isConnected(a_toInputPort)) {
            const size_t nRecords = m_connectionRecords.size();
            for (int i = 0; i < nRecords; i++) {
                const ConnectionRecord& rec = m_connectionRecords[i];
                if (rec.to_id == a_toId && rec.to_port == a_toInputPort) {
                    m_connectionRecords.erase(m_connectionRecords.begin() + i);
                    break;
                }
            }
        }

        // make new connection
        toUnit->connectInput(a_toInputPort, &fromUnit->readOutput(a_fromOutputPort, 0));

        // record the connection upon success
        m_connectionRecords.push_back({a_fromId, a_fromOutputPort, a_toId,a_toInputPort});
        _recomputeGraph();
        return true;
    }

    bool Circuit::connectInternal(const Unit& a_fromUnit, int a_fromOutputPort, const Unit& a_toUnit, int a_toInputPort) {
        const Unit* toUnitPtr = &a_toUnit;
        const Unit* fromUnitPtr = &a_fromUnit;
        int fromId = m_units.getIdFromItem(toUnitPtr);
        int toId = m_units.getIdFromItem(fromUnitPtr);
        return connectInternal(fromId, a_fromOutputPort, toId, a_toInputPort);
    }

    bool Circuit::disconnectInternal(const Unit& a_fromUnit, int a_fromOutputPort, const Unit& a_toUnit, int a_toInputPort) {
        const Unit* toUnitPtr = &a_toUnit;
        const Unit* fromUnitPtr = &a_fromUnit;
        int fromId = m_units.getIdFromItem(toUnitPtr);
        int toId = m_units.getIdFromItem(fromUnitPtr);
        return disconnectInternal(fromId, a_fromOutputPort, toId, a_toInputPort);
    }

    const vector<ConnectionRecord>& Circuit::getConnections() const { return m_connectionRecords; };

    void Circuit::_recomputeGraph()
    {
        list<int> sinks;
        // Find sinks
        for (int i = 0; i < m_units.size(); i++)
        {
            const Unit* unit = m_units.getByIndex(i);
            if (!unit->numOutputs()) {
                sinks.push_back(m_units.getIdFromItem(unit));
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

    void Circuit::setVoiceIndex(double a_newVoiceIndex) { m_voiceIndex = a_newVoiceIndex; }

    double Circuit::getVoiceIndex() const { return m_voiceIndex; }

    void Circuit::onFsChange_()
    {
        const int* unitIndices = m_units.ids();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->setFs(fs()); }
    }

    void Circuit::onTempoChange_()
    {
        const int* unitIndices = m_units.ids();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->setTempo(tempo()); }
    }

    void Circuit::onNoteOn_()
    {
        const int* unitIndices = m_units.ids();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->noteOn(note(), velocity()); }
    }

    void Circuit::onNoteOff_()
    {
        const int* unitIndices = m_units.ids();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->noteOff(note(), velocity()); }
    }

    void Circuit::onMidiControlChange_(int a_cc, double a_value)
    {
        const int* unitIndices = m_units.ids();
        for (int i = 0; i < m_units.size(); i++) { m_units[unitIndices[i]]->onMidiControlChange_(a_cc, a_value); }
    }

    void Circuit::onInputConnection_(int a_inputPort) { m_inputUnit->connectInput(a_inputPort, &readInput(a_inputPort, 0)); }

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

    int Circuit::getInputUnitId() const { return getUnitId(*m_inputUnit); }

    int Circuit::getOutputUnitId() const { return getUnitId(*m_outputUnit); }

    int Circuit::getNumUnits() const { return static_cast<int>(m_units.size()); }

    Unit* const* Circuit::getProcGraph() const { return m_procGraph.data(); }

    void Circuit::notifyMidiControlChange(int a_cc, double a_value) { onMidiControlChange_(a_cc, a_value); }

    void Circuit::setBufferSize(int a_bufferSize)
    {
        Unit::setBufferSize(a_bufferSize);

        /* Update buffer sizes of internal units */
        const int* unitIndices = m_units.ids();
        for (int i = 0; i < m_units.size(); i++)
        {
            m_units[unitIndices[i]]->setBufferSize(a_bufferSize);
        }

        /* Update internal connections, since the location of the unit output buffers may have changed */
        for (const auto& cr : m_connectionRecords)
        {
            Unit* fromUnit = m_units[cr.from_id];
            Unit* toUnit = m_units[cr.to_id];
            toUnit->connectInput(cr.to_port, &fromUnit->readOutput(cr.from_port, 0));
        }
    }

    vector<std::pair<int, int>> Circuit::getConnectionsToInternalInput(int a_unitId, int a_portid) const
    {
        vector<std::pair<int, int>> connectedPorts;
        for (int i = 0; i < m_connectionRecords.size(); i++)
        {
            const ConnectionRecord& conn = m_connectionRecords[i];
            if (conn.to_id == a_unitId && conn.to_port == a_portid) { connectedPorts.push_back(std::make_pair(conn.from_id, conn.from_port)); }
        }
        return connectedPorts;
    }
}
