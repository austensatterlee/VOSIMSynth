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

#ifndef __Circuit__
#define __Circuit__
#include "vosimlib/Unit.h"
#include "vosimlib/IntMap.h"
#include <vector>
#include <list>

using std::vector;
using std::list;

#define MAX_UNITS 128

namespace syn
{
    struct VOSIMLIB_API ConnectionRecord
    {
        int from_id;
        int from_port;
        int to_id;
        int to_port;

        ConnectionRecord(int a_fromId, int a_fromPort, int a_toId, int a_toPort)
            : from_id(a_fromId),
              from_port(a_fromPort),
              to_id(a_toId),
              to_port(a_toPort) {}

        ConnectionRecord()
            : ConnectionRecord(-1, -1, -1, -1) {}

        bool operator==(const ConnectionRecord& a_other) const
        {
            bool res = (from_id == a_other.from_id && from_port == a_other.from_port && to_id == a_other.to_id && to_port == a_other.to_port);
            return res;
        }
    };

    class VOSIMLIB_API PassthroughUnit : public Unit
    {
    public:
        explicit PassthroughUnit(const string& a_name) :
            Unit(a_name) { }

        PassthroughUnit(const PassthroughUnit& a_other) :
            PassthroughUnit(a_other.name()) { };

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override
        {
            BEGIN_PROC_FUNC
            for (int i = 0; i < numInputs(); i++)
            {
                WRITE_OUTPUT(i, READ_INPUT(i));
            }
            END_PROC_FUNC
        }
    };

    class VOSIMLIB_API InputUnit : public PassthroughUnit
    {
        DERIVE_UNIT(InputUnit)
    public:
        explicit InputUnit(const string& a_name) : PassthroughUnit(a_name) {}
        InputUnit(const InputUnit& a_other) : InputUnit(a_other.name()) {}
    };

    class VOSIMLIB_API OutputUnit : public PassthroughUnit
    {
        DERIVE_UNIT(OutputUnit)
    public:
        explicit OutputUnit(const string& a_name) : PassthroughUnit(a_name) {}
        OutputUnit(const OutputUnit& a_other) : OutputUnit(a_other.name()) {}
    };

    /**
    * \class Circuit
    *
    * \brief A collection of Units.
    *
    * A Circuit is a Unit that contains other Units.
    *
    */
    class VOSIMLIB_API Circuit : public Unit
    {
        DERIVE_UNIT(Circuit)
    public:
        explicit Circuit(const string& a_name);

        Circuit(const Circuit& a_other);

        Circuit& operator=(const Circuit& a_other);

        virtual ~Circuit();

        /**
         * \returns True if any of its internal units are active
         */
        bool isActive() const override;

        void setVoiceIndex(double a_newVoiceIndex);
        double getVoiceIndex() const;

        Unit& getUnit(int a_id);
        const Unit& getUnit(int a_id) const;

        int getInputUnitId() const;
        int getOutputUnitId() const;   

        int getUnitId(const Unit& a_unit) const { const Unit* unitPtr = &a_unit; return m_units.getIdFromItem(unitPtr); }
        int getUnitId(int a_index) const { return m_units.getIdFromIndex(a_index); }

        int getNumUnits() const;
        const auto& getUnits() const { return m_units; }
        Unit* const* getProcGraph() const;

        void notifyMidiControlChange(int a_cc, double a_value);

        void setBufferSize(int a_bufferSize) override;

        /**
         * Retrieves a list of output ports connected to an input port.
         * \returns A vector of (unit_id, port_id) pairs.
         */
        vector<std::pair<int, int>> getConnectionsToInternalInput(int a_id, int a_portid) const;
        
        /**
         * Get a list of all connections within the Circuit.
         */
        const vector<ConnectionRecord>& getConnections() const;

        /**
         * \returns The id assigned to the newly added unit, or -1 on failure
         */
        int addUnit(Unit* a_unit);

        /**
         * \returns True if the unit was added to the circuit (i.e. the provided id was not already taken).
         */
        bool addUnit(Unit* a_unit, int a_id);

        bool removeUnit(const Unit& a_unit);
        bool removeUnit(int a_id);

        bool connectInternal(int a_fromId, int a_fromOutputPort, int a_toId, int a_toInputPort);
        bool connectInternal(const Unit& a_fromUnit, int a_fromOutputPort, const Unit& a_toUnit, int a_toInputPort);

        bool disconnectInternal(int a_fromId, int a_fromOutputPort, int a_toId, int a_toInputPort);
        bool disconnectInternal(const Unit& a_fromUnit, int a_fromOutputPort, const Unit& a_toUnit, int a_toInputPort);

        operator json() const override;

        Unit* load(const json& j) override;

        void reset() override;

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;

        void onFsChange_() override;

        void onTempoChange_() override;

        void onNoteOn_() override;

        void onNoteOff_() override;

        void onMidiControlChange_(int a_cc, double a_value) override;

        void onInputConnection_(int a_inputPort) override;

        void onInputDisconnection_(int a_inputPort) override;

        int addExternalInput_(const string& a_name);

        int addExternalOutput_(const string& a_name);

    private:
        void _recomputeGraph();

    private:
        friend class VoiceManager;

        double m_voiceIndex; ///< A number between 0 and 1 assigned to the circuit by a VoiceManager.
        IntMap<Unit*, MAX_UNITS> m_units;
        vector<ConnectionRecord> m_connectionRecords;
        InputUnit* m_inputUnit;
        OutputUnit* m_outputUnit;

        array<Unit*, MAX_UNITS> m_procGraph;
    };
};

#endif // __Circuit__
