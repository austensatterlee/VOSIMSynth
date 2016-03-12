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

using std::vector;
using std::unordered_set;

namespace syn
{


    Circuit::Circuit() :
    Circuit("")
    {}

    Circuit::Circuit(const string& a_name) :
            Unit(a_name)
    {
        addInput_("left in");
        addInput_("right in");
        addOutput_("left out");
        addOutput_("right out");
    }

    Circuit::Circuit(const Circuit& a_other) :
            Circuit(a_other.getName())
    {
        for(int i=0;i<a_other.m_units.size();i++){
            addUnit(shared_ptr<Unit>(a_other.m_units[i]->clone()));
        }
        for(int i=0;i<a_other.m_connectionRecords.size();i++){
            const ConnectionRecord& rec = a_other.m_connectionRecords[i];
            if(rec.type == ConnectionRecord::Input){
                connectInputs(rec.from_port, rec.to_id, rec.to_port);
            }else if(rec.type == ConnectionRecord::Output){
                connectOutputs(rec.from_port, rec.to_id, rec.to_port);
            }else if(rec.type == ConnectionRecord::Internal){
                connectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
            }
        }
    }

	void Circuit::process_(const SignalBus& inputs, SignalBus& outputs)
	{
		// reset all internal components
		for (int i = 0; i < m_units.size(); i++)
		{
			m_units[i]->reset();
		}

        /* Push circuit input signals to internally connected input ports */
        for(int i=0;i< m_inputSignals.getNumChannels(); i++){
            Signal& inputSignal = m_inputSignals.getChannel(i);
            m_externalInputs.push(i, inputSignal);
        }

		// tick all internal components
		for (int i = 0; i < m_units.size(); i++)
		{
			m_units[i]->tick();
		}

        /* Push internally connected output signals to circuit output ports */
        for(int i=0;i< m_outputSignals.getNumChannels(); i++){
            Signal& outputSignal = m_outputSignals.getChannel(i);
            m_externalOutputs.pull(i, outputSignal);
        }
	}

    int Circuit::addUnit(shared_ptr<Unit> a_unit)
    {
        int retval = m_units.add(a_unit.get()->getName(),a_unit);
        if(retval<0)
            return retval;
        a_unit->_setParent(this);
        a_unit->setFs(getFs());
        a_unit->setTempo(getTempo());
        a_unit->m_midiData = m_midiData;
        return retval;
    }

    vector<pair<int, int> > Circuit::getConnectionsToCircuitInput(int a_circuitInputPort) const{
        vector<pair<int, int> > connectedPorts;
        for(int i=0;i<m_connectionRecords.size();i++){
            const ConnectionRecord& conn = m_connectionRecords[i];
            if(conn.type==ConnectionRecord::Input && conn.from_port == a_circuitInputPort){
                connectedPorts.push_back(make_pair(conn.to_id,conn.to_port));
            }
        }
        return connectedPorts;
    };

    vector<pair<int, int> > Circuit::getConnectionsToCircuitOutput(int a_circuitOutputPort) const{
        vector<pair<int, int> > connectedPorts;
        for(int i=0;i<m_connectionRecords.size();i++){
            const ConnectionRecord& conn = m_connectionRecords[i];
            if(conn.type==ConnectionRecord::Output && conn.from_port == a_circuitOutputPort){
                connectedPorts.push_back(make_pair(conn.to_id,conn.to_port));
            }
        }
        return connectedPorts;
    }

	const vector<ConnectionRecord>& Circuit::getConnectionRecords() const {
		return m_connectionRecords;
    };

    Unit* Circuit::_clone() const
    {
        return new Circuit(*this);
    }

    bool Circuit::isActive() const
    {
        for(int i=0;i<m_units.size();i++){
            if (m_units[i]->isActive())
                return true;
        }
        return false;
    }

    void Circuit::onFsChange_()
    {
        for(int i=0;i<m_units.size();i++){
            m_units[i]->setFs(getFs());
        }
    }

    void Circuit::onTempoChange_()
    {
        for(int i=0;i<m_units.size();i++){
            m_units[i]->setTempo(getTempo());
        }
    }

    void Circuit::onNoteOn_()
    {
        for(int i=0;i<m_units.size();i++){
            m_units[i]->noteOn(getNote(),getVelocity());
        }
    }

    void Circuit::onNoteOff_()
    {
        for(int i=0;i<m_units.size();i++){
            m_units[i]->noteOff(getNote(),getVelocity());
        }
    }

	Unit& Circuit::getUnit_(int a_unitId) {
		return *m_units[a_unitId];
    }

	const Unit& Circuit::getUnit(int a_unitId) const
    {
        return *m_units[a_unitId];
    }

    int Circuit::getNumUnits() const
    {
        return m_units.size();
    }
}

