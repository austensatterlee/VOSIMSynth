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
            Unit(a_name),
			m_units(256)
    {
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
            Circuit(a_other.getName())
    {
        for(int i=0;i<a_other.m_units.size();i++){
            addUnit(shared_ptr<Unit>(a_other.m_units[i]->clone()));
        }
        for(int i=0;i<a_other.m_connectionRecords.size();i++){
            const ConnectionRecord& rec = a_other.m_connectionRecords[i];
            connectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);            
        }
    }

	void Circuit::process_(const SignalBus& inputs, SignalBus& outputs)
	{
		// reset all internal components
		for (int i = 0; i < m_units.size(); i++)
		{
			m_units[i]->reset();
		}

		/* Push external input data into internal input unit */
		m_inputUnit->m_inputSignals.set(m_inputSignals);

		// tick all internal components
		for (int i = 0; i < m_units.size(); i++)
		{
			m_units[i]->tick();
		}

        /* Push internally connected output signals to circuit output ports */
		m_outputSignals.set(m_outputUnit->m_outputSignals);
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

	bool Circuit::addUnit(shared_ptr<Unit> a_unit, int a_unitId) {
		bool retval = m_units.add(a_unit.get()->getName(), a_unitId, a_unit);
		if (!retval)
			return false;
		a_unit->_setParent(this);
		a_unit->setFs(getFs());
		a_unit->setTempo(getTempo());
		a_unit->m_midiData = m_midiData;
		return true;
    }

	const vector<ConnectionRecord>& Circuit::getConnections() const {
		return m_connectionRecords;
    };

    Unit* Circuit::_clone() const
    {
        return new Circuit(*this);
    }

    bool Circuit::isActive() const
    {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
            if (it->second->isActive())
                return true;
        }
        return false;
    }

    void Circuit::onFsChange_()
    {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
            it->second->setFs(getFs());
        }
    }

    void Circuit::onTempoChange_()
    {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
           it->second->setTempo(getTempo());
        }
    }

    void Circuit::onNoteOn_()
    {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
            it->second->noteOn(getNote(),getVelocity());
        }
    }

    void Circuit::onNoteOff_()
    {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
            it->second->noteOff(getNote(),getVelocity());
        }
    }

	void Circuit::onMidiControlChange_(int a_cc, double a_value) {
		auto units = m_units.data();
		for (auto it = units.begin(); it != units.end(); ++it) {
			it->second->onMidiControlChange_(a_cc, a_value);
		}
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

	int Circuit::getNumUnits() const
    {
        return m_units.size();
    }

	void Circuit::notifyMidiControlChange(int a_cc, double a_value) {
		onMidiControlChange_(a_cc, a_value);
    }
}

