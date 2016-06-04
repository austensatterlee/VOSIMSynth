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

#include "Unit.h"

using std::hash;

using namespace std;

namespace syn
{
	Unit::Unit() :
		m_name{ "" },
		m_parent{ nullptr },
		m_audioConfig{},
		m_midiData{} { }

	Unit::Unit(const string& a_name) :
		m_name{ a_name },
		m_parent{ nullptr },
		m_audioConfig{},
		m_midiData{} { }

	unsigned int Unit::getClassIdentifier() const {
		hash<string> hash_fn;
		return hash_fn(_getClassName());
	}

	void Unit::setFs(double a_newFs) {
		m_audioConfig.fs = a_newFs;
		onFsChange_();
	}

	void Unit::setTempo(double a_newTempo) {
		m_audioConfig.tempo = a_newTempo;
		onTempoChange_();
	}

	void Unit::noteOn(int a_note, int a_velocity) {
		m_midiData.note = a_note;
		m_midiData.velocity = a_velocity;
		m_midiData.isNoteOn = true;
		onNoteOn_();
	}

	void Unit::noteOff(int a_note, int a_velocity) {
		m_midiData.isNoteOn = false;
		onNoteOff_();
	}

	double Unit::getFs() const {
		return m_audioConfig.fs;
	}

	double Unit::getTempo() const {
		return m_audioConfig.tempo;
	}

	bool Unit::isNoteOn() const {
		return m_midiData.isNoteOn;
	}

	int Unit::getNote() const {
		return m_midiData.note;
	}

	int Unit::getVelocity() const {
		return m_midiData.velocity;
	}

	int Unit::getNumParameters() const {
		return m_parameters.size();
	}

	int Unit::getNumInputs() const {
		return m_inputPorts.size();
	}

	int Unit::getNumOutputs() const {
		return m_outputSignals.size();
	}

	const string& Unit::getName() const {
		return m_name;
	}

	void Unit::_setName(const string& a_name) {
		m_name = a_name;
	}

	bool Unit::isActive() const {
		return m_midiData.isNoteOn;
	}

	const Circuit* Unit::getParent() const {
		return m_parent;
	}

	void Unit::tick() {
		// Clear outputs
		for (int id : m_outputSignals.getIds()) {
			m_outputSignals[id] = 0.0;
		}

		process_();
	}

	int Unit::addInput_(const string& a_name, double a_default) {
		return m_inputPorts.add(a_name, { a_default });
	}

	int Unit::addOutput_(const string& a_name) {
		return m_outputSignals.add(a_name, 0);
	}

	int Unit::addParameter_(const UnitParameter& a_param) {
		return m_parameters.add(a_param.getName(), a_param);
	}

	void Unit::_setParent(Circuit* a_new_parent) {
		m_parent = a_new_parent;
	}

	void Unit::connectInput(int a_inputPort, const double* a_src)
	{
		m_inputPorts[a_inputPort].src = a_src;
		onInputConnection_(a_inputPort);
	}

	bool Unit::isConnected(int a_inputPort) const
	{
		return m_inputPorts[a_inputPort].src != nullptr;
	}

	bool Unit::disconnectInput(int a_inputPort) {
		bool retval = m_inputPorts[a_inputPort].src == nullptr;
		m_inputPorts[a_inputPort].src = nullptr;
		if (retval) onInputDisconnection_(a_inputPort);
		return retval;
	}

	Unit* Unit::clone() const {
		Unit* unit = _clone();

		unit->m_name = m_name;
		unit->m_outputSignals = m_outputSignals;
		unit->m_parameters = m_parameters;
		unit->m_midiData = m_midiData;
		unit->setFs(m_audioConfig.fs);
		unit->setTempo(m_audioConfig.tempo);
		return unit;
	}

	const double& Unit::getInputValue(int a_index) const {
		return m_inputPorts[a_index].src == nullptr ? m_inputPorts[a_index].defVal : *m_inputPorts[a_index].src;
	}

	string Unit::getInputName(int a_index) const {
		return m_inputPorts.getItemName(a_index);
	}
}