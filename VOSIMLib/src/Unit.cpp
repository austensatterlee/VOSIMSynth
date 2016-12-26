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
#include "DSPMath.h"

using std::hash;

namespace syn
{
    Unit::Unit() : Unit("") {}

    Unit::Unit(const std::string& a_name) :
        m_name{a_name},
        m_parent{nullptr},
        m_audioConfig{44.1e3, 120},
        m_midiData{} { }

    void Unit::setName(const string& a_name){ m_name = a_name; }

    unsigned int Unit::getClassIdentifier() const
    {
        hash<string> hash_fn;
        return static_cast<unsigned int>(hash_fn(_getClassName()));
    }

    void Unit::setFs(double a_newFs)
    {
        m_audioConfig.fs = a_newFs;
        reset();
        onFsChange_();
    }

    void Unit::setTempo(double a_newTempo)
    {
        m_audioConfig.tempo = a_newTempo;
        onTempoChange_();
    }

    void Unit::noteOn(int a_note, int a_velocity)
    {
        m_midiData.note = a_note;
        m_midiData.velocity = a_velocity;
        m_midiData.isNoteOn = true;
        reset();
        onNoteOn_();
    }

    void Unit::noteOff(int a_note, int a_velocity)
    {
        m_midiData.isNoteOn = false;
        onNoteOff_();
    }

    void Unit::notifyParameterChanged(int a_id) { onParamChange_(a_id); }

    double Unit::fs() const { return m_audioConfig.fs; }

    double Unit::tempo() const { return m_audioConfig.tempo; }

    bool Unit::isNoteOn() const { return m_midiData.isNoteOn; }

    int Unit::note() const { return m_midiData.note; }

    int Unit::velocity() const { return m_midiData.velocity; }

    const NamedContainer<double, 8>& Unit::outputs() const { return m_outputSignals; }

    int Unit::numParams() const { return static_cast<int>(m_parameters.size()); }

    int Unit::numInputs() const { return static_cast<int>(m_inputPorts.size()); }

    int Unit::numOutputs() const { return static_cast<int>(m_outputSignals.size()); }

    const string& Unit::name() const { return m_name; }

    void Unit::_setName(const string& a_name) { m_name = a_name; }

    bool Unit::isActive() const { return m_midiData.isNoteOn; }

    const Circuit* Unit::parent() const { return m_parent; }

    const string& Unit::paramName(int a_index) const { return m_parameters.name(a_index); }

    const NamedContainer<UnitParameter, MAX_PARAMS>& Unit::parameters() const { return m_parameters; }

    void Unit::tick() { process_(); }

    void Unit::tick(const Eigen::Array<double, -1, -1, Eigen::RowMajor>& a_inputs, Eigen::Array<double, -1, -1, Eigen::RowMajor>& a_outputs)
    {
        const double* oldSources[MAX_INPUTS];
        int nSamples = a_inputs.cols();
        int nInputs = MIN<int>(a_inputs.rows(), static_cast<int>(m_inputPorts.size()));
        // record original input sources
        for (int i = 0; i < nInputs; i++) { oldSources[i] = m_inputPorts.getByIndex(i).src; }
        for (int i = 0; i < nSamples; i++)
        {
            // nudge input sources by one sample
            for (int j = 0; j < nInputs; j++) { m_inputPorts.getByIndex(j).src = &a_inputs(j, i); }

            tick();

            // record outputs
            for (int j = 0; j < m_outputSignals.size(); j++)
                a_outputs(j, i) = m_outputSignals.getByIndex(j);
        }
        // restore original input sources
        for (int i = 0; i < nInputs; i++) { m_inputPorts.getByIndex(i).src = oldSources[i]; }
    }

    int Unit::addInput_(const string& a_name, double a_default) { return m_inputPorts.add(a_name, {a_default}); }

    bool Unit::addInput_(int a_id, const string& a_name, double a_default) { return m_inputPorts.add(a_name, a_id, {a_default}); }

    bool Unit::addOutput_(int a_id, const string& a_name) { return m_outputSignals.add(a_name, a_id, 0.0); }

    bool Unit::addParameter_(int a_id, const UnitParameter& a_param)
    {
        bool retval = m_parameters.add(a_param.getName(), a_id, a_param);
        if (retval)
        {
            m_parameters[a_id].setParent(this);
            m_parameters[a_id].setId(a_id);
        }
        return retval;
    }

    int Unit::addOutput_(const string& a_name) { return m_outputSignals.add(a_name, 0.0); }

    int Unit::addParameter_(const UnitParameter& a_param)
    {
        int id = m_parameters.add(a_param.getName(), a_param);
        if (id >= 0)
        {
            m_parameters[id].setParent(this);
            m_parameters[id].setId(id);
        }
        return id;
    }

    void Unit::_setParent(Circuit* a_new_parent) { m_parent = a_new_parent; }

    void Unit::connectInput(int a_inputPort, const double* a_src)
    {
        m_inputPorts[a_inputPort].src = a_src;
        onInputConnection_(a_inputPort);
    }

    bool Unit::isConnected(int a_inputPort) const { return m_inputPorts[a_inputPort].src != nullptr; }

    bool Unit::disconnectInput(int a_inputPort)
    {
        bool retval = isConnected(a_inputPort);
        m_inputPorts[a_inputPort].src = nullptr;
        if (retval)
            onInputDisconnection_(a_inputPort);
        return retval;
    }

    void Unit::copyFrom_(const Unit& a_other)
    {
        // Copy name
        m_name = a_other.m_name;
        // Copy midi status
        m_midiData = a_other.m_midiData;
        // Copy audio config data
        setFs(a_other.m_audioConfig.fs);
        setTempo(a_other.m_audioConfig.tempo);
        // Copy parameter values
        const string* paramNames = m_parameters.names();
        for (int i = 0; i < m_parameters.size(); i++)
        {
            const string& paramName = paramNames[i];
            if (a_other.hasParam(paramName))
                m_parameters[paramName].setFromString(a_other.m_parameters[paramName].getValueString());
        }
    }

    Unit* Unit::clone() const
    {
        Unit* unit = _clone();
        unit->copyFrom_(*this);
        return unit;
    }

    Unit::operator json() const
    {
        json j;
        j["name"] = m_name;
        /* Serialize parameters */
        j["parameters"] = json();
        for (int i = 0; i < m_parameters.size(); i++)
        {
            int index = m_parameters.indices()[i];
            j["parameters"][std::to_string(index)] = m_parameters[index];
        }
        j["class_id"] = getClassIdentifier();
        return j;
    }

    Unit* Unit::fromJSON(const json& j)
    {
        // Construct new Unit
        unsigned class_id = j["class_id"].get<unsigned>();
        std::string name = j["name"];
        Unit* unit = UnitFactory::instance().createUnit(class_id, name);

        // Load saved parameters into the new unit
        json params = j["parameters"];
        for (json::iterator it = params.begin(); it != params.end(); ++it)
        {
            int index = stoi(it.key());
            unit->m_parameters[index].load(j["parameters"][it.key()]);
        }

        unit->load(j);
        return unit;
    }

    const double& Unit::readInput(int a_index) const { return m_inputPorts[a_index].src ? *m_inputPorts[a_index].src : m_inputPorts[a_index].defVal; }

    const double* Unit::inputSource(int a_index) const { return m_inputPorts[a_index].src; }

    const NamedContainer<UnitPort, MAX_INPUTS>& Unit::inputs() const { return m_inputPorts; }

    bool Unit::hasOutput(int a_outputPort) const { return m_outputSignals.contains(a_outputPort); }

    bool Unit::hasInput(int a_inputPort) const { return m_inputPorts.contains(a_inputPort); }

    const string& Unit::inputName(int a_index) const { return m_inputPorts.name(a_index); }
}
