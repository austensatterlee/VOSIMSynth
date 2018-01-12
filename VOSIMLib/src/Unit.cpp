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
#include "vosimlib/Unit.h"
#include "vosimlib/DSPMath.h"

using std::hash;

namespace syn
{
    Unit::Unit() : Unit("") {}

    Unit::Unit(const std::string& a_name) :
        m_currentBufferOffset(0),
        m_name{a_name},
        m_parent{nullptr},
        m_audioConfig{44.1e3, 120, 1},
        m_midiData{} {}

    void Unit::setName(const string& a_name) { m_name = a_name; }

    UnitTypeId Unit::getClassIdentifier() const
    {
        hash<string> hash_fn;
        return static_cast<UnitTypeId>(hash_fn(getClassName()));
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
        onNoteOn_();
    }

    void Unit::noteOff(int a_note)
    {
        m_midiData.isNoteOn = false;
        onNoteOff_();
    }

    void Unit::setBufferSize(int a_bufferSize)
    {
        m_audioConfig.bufferSize = a_bufferSize;
        for(auto& output : m_outputPorts) {
            output.resize(a_bufferSize);
        }
    }

    int Unit::getBufferSize() const { return m_audioConfig.bufferSize; }

    void Unit::notifyParameterChanged(int a_id) { onParamChange_(a_id); }

    double Unit::fs() const { return m_audioConfig.fs; }

    double Unit::tempo() const { return m_audioConfig.tempo; }

    bool Unit::isNoteOn() const { return m_midiData.isNoteOn; }

    int Unit::note() const { return m_midiData.note; }

    int Unit::velocity() const { return m_midiData.velocity; }

    double* Unit::outputSource(int a_id) { return m_outputPorts[a_id].buf(); }

    const StrMap<OutputPort, MAX_OUTPUTS>& Unit::outputs() const { return m_outputPorts; }

    int Unit::numParams() const { return static_cast<int>(m_parameters.size()); }

    int Unit::numInputs() const { return static_cast<int>(m_inputPorts.size()); }

    int Unit::numOutputs() const { return static_cast<int>(m_outputPorts.size()); }

    bool Unit::isActive() const { return m_midiData.isNoteOn; }

    const Circuit* Unit::parent() const { return m_parent; }

    const string& Unit::paramName(int a_id) const { return m_parameters.getNameFromId(a_id); }

    const StrMap<UnitParameter, MAX_PARAMS>& Unit::parameters() const { return m_parameters; }

    void Unit::tick(const Eigen::Array<double, -1, -1, Eigen::RowMajor>& a_inputs, Eigen::Array<double, -1, -1, Eigen::RowMajor>& a_outputs)
    {
        const double* oldInputSources[MAX_INPUTS];
        double* oldOutputSources[MAX_OUTPUTS];
        int nSamples = a_inputs.cols();
        int nInputs = MIN<int>(a_inputs.rows(), m_inputPorts.size());
        int nOutputs = MIN<int>(a_outputs.rows(), m_outputPorts.size());
        // set new buffer size
        int oldBufferSize = m_audioConfig.bufferSize;
        setBufferSize(nSamples);

        // record original input sources
        for (int i = 0; i < nInputs; i++) { oldInputSources[i] = m_inputPorts.getByIndex(i).src; }
        // set new input sources
        for (int i = 0; i < nInputs; i++) { m_inputPorts.getByIndex(i).src = &a_inputs(i, 0); }       
        // record original output sources
        for (int i = 0; i < nOutputs; i++) { oldOutputSources[i] = m_outputPorts.getByIndex(i).buf(); }
        // set new output sources
        for (int i = 0; i < nOutputs; i++) { m_outputPorts.getByIndex(i).connect(&a_outputs(i,0)); }

        tick();

        // restore original buffer size
        setBufferSize(oldBufferSize);

        // restore original input sources
        for (int i = 0; i < nInputs; i++) { m_inputPorts.getByIndex(i).src = oldInputSources[i]; }
        // restore original output sources
        for (int i = 0; i < nOutputs; i++) { m_outputPorts.getByIndex(i).connect(oldOutputSources[i]); }
    }

    int Unit::addInput_(const string& a_name, double a_default) { return m_inputPorts.add(a_name, InputPort{a_default}); }

    bool Unit::addInput_(int a_id, const string& a_name, double a_default) { return m_inputPorts.add(a_name, a_id, InputPort{a_default}); }
    
    bool Unit::removeInput_(const string& a_name) { return m_inputPorts.removeByName(a_name); }
    bool Unit::removeInput_(int a_id) { return m_inputPorts.removeById(a_id); }

    bool Unit::addOutput_(int a_id, const string& a_name) { return m_outputPorts.add(a_name, a_id, OutputPort(getBufferSize())); }

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

    int Unit::addOutput_(const string& a_name) { return m_outputPorts.add(a_name, OutputPort(getBufferSize())); }

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

    bool Unit::isInputConnected(int a_inputPort) const { return m_inputPorts[a_inputPort].src != nullptr; }

    bool Unit::disconnectInput(int a_inputPort)
    {
        bool retval = isInputConnected(a_inputPort);
        m_inputPorts[a_inputPort].src = nullptr;
        if (retval)
            onInputDisconnection_(a_inputPort);
        return retval;
    }

    void Unit::connectOutput(int a_outputPort, double* a_dst) {
        m_outputPorts[a_outputPort].connect(a_dst);
    }

    bool Unit::isOutputConnected(int a_outputPort) const {
        return m_outputPorts[a_outputPort].isConnected();
    }

    bool Unit::disconnectOutput(int a_outputPort) {
        bool wasConnected = isOutputConnected(a_outputPort);
        m_outputPorts[a_outputPort].disconnect();
        return wasConnected;
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
        setBufferSize(a_other.m_audioConfig.bufferSize);
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
            int index = m_parameters.ids()[i];
            j["parameters"][std::to_string(index)] = m_parameters[index];
        }
        j["class_id"] = getClassIdentifier();
        return j;
    }

    Unit* Unit::fromJSON(const json& j)
    {
        // Construct new Unit
        UnitTypeId class_id = j["class_id"].get<UnitTypeId>();
        std::string name = j["name"].get<std::string>();
        Unit* unit = UnitFactory::instance().createUnit(class_id, name);
        if (!unit)
            return nullptr;

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

    const double& Unit::readInput(int a_id, int a_offset) const { return m_inputPorts[a_id].src ? *(m_inputPorts[a_id].src + a_offset) : m_inputPorts[a_id].defVal; }

    const double* Unit::inputSource(int a_id) const { return m_inputPorts[a_id].src; }

    const StrMap<InputPort, MAX_INPUTS>& Unit::inputs() const { return m_inputPorts; }

    bool Unit::hasOutput(int a_id) const { return m_outputPorts.containsId(a_id); }

    string Unit::outputName(int a_id) const { return m_outputPorts.getNameFromId(a_id); }

    bool Unit::hasInput(int a_id) const { return m_inputPorts.containsId(a_id); }

    const string& Unit::inputName(int a_id) const { return m_inputPorts.getNameFromId(a_id); }
}
