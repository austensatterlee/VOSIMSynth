#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include "Unit.h"
#include "StateVariableFilter.h"
#include <string>
#include "Circuit.h"
#include "Oscillator.h"
#include "VosimOscillator.h"
#include "ADSREnvelope.h"
#include "MemoryUnit.h"
#include "MidiUnits.h"
#include "MathUnits.h"
#include "WaveShapers.h"
#include "Follower.h"

namespace py = pybind11;

template <class Base = syn::Unit>
class PyUnit_tpl : public Base
{
public:
    using Base::Base; // Inherit constructors
    void reset() override
    {
        PYBIND11_OVERLOAD(void, Base, reset, );
    }

    bool isActive() const override
    {
        PYBIND11_OVERLOAD(bool, Base, isActive, );
    }

    void onParamChange_(int a_paramId) override
    {
        PYBIND11_OVERLOAD(void, Base, onParamChange_, a_paramId);
    };

    void onFsChange_() override
    {
        PYBIND11_OVERLOAD(void, Base, onFsChange_, );
    }

    void onTempoChange_() override
    {
        PYBIND11_OVERLOAD(void, Base, onTempoChange_, );
    }

    void onNoteOn_() override
    {
        PYBIND11_OVERLOAD(void, Base, onNoteOn_, );
    }

    void onNoteOff_() override
    {
        PYBIND11_OVERLOAD(void, Base, onNoteOff_, );
    }

    void onMidiControlChange_(int a_cc, double a_value) override
    {
        PYBIND11_OVERLOAD(void, Base, onMidiControlChange_, a_cc, a_value);
    }

    void onInputConnection_(int a_inputPort) override
    {
        PYBIND11_OVERLOAD(void, Base, onInputConnection_, a_inputPort);
    }

    void onInputDisconnection_(int a_inputPort) override
    {
        PYBIND11_OVERLOAD(void, Base, onInputDisconnection_, a_inputPort);
    }

    void MSFASTCALL process_() GCCFASTCALL override
    {
        PYBIND11_OVERLOAD_PURE(void, Base, process_, );
    }

    inline string getClassName() const override
    {
        PYBIND11_OVERLOAD_PURE(std::string, Base, getClassName, );
    }

    syn::Unit* _clone() const override
    {
        PYBIND11_OVERLOAD_PURE(syn::Unit*, Base, _clone, );
    }
};

PYBIND11_PLUGIN(pyVOSIMLib)
{
    py::module m("pyVOSIMLib", "Python bindings for VOSIMLib");

    py::class_<syn::Unit, std::shared_ptr<syn::Unit>,
               PyUnit_tpl<syn::Unit>>(m, "Unit")
            .def(py::init<const std::string &>())
            .def("tick", [](syn::Unit& self, const Eigen::Array<double, -1, -1, Eigen::RowMajor>& a_inputs)
                {
                    if (a_inputs.rows() > self.numInputs())
                        throw std::runtime_error("Input buffer should have " + std::to_string(self.numOutputs()) + " rows.");
                    Eigen::Array<double, -1, -1, Eigen::RowMajor> outputs(self.numOutputs(), a_inputs.cols());
                    self.tick(a_inputs, outputs);
                    return outputs;
                }
            )
            .def("reset", &syn::Unit::reset)
            .def_property_readonly("is_active", &syn::Unit::isActive)

            .def_property("fs", &syn::Unit::fs, &syn::Unit::setFs)
            .def_property("tempo", &syn::Unit::tempo, &syn::Unit::setTempo)
            .def_property("bufferSize", &syn::Unit::getBufferSize, &syn::Unit::setBufferSize)

            .def_property_readonly("note", &syn::Unit::note)
            .def_property_readonly("is_note_on", &syn::Unit::isNoteOn)
            .def("note_on", &syn::Unit::noteOn)
            .def("note_off", &syn::Unit::noteOff)
            .def_property_readonly("velocity", &syn::Unit::velocity)

            .def("set_param", &syn::Unit::setParam<string, double>)
            .def("set_param_norm", [](syn::Unit& self, const std::string& a_paramName, double a_value)
                {
                    return self.param(a_paramName).setNorm(a_value);
                })
            .def_property_readonly("params", [](const syn::Unit& self)
                {
                    std::map<string, double> parameters;
                    auto nc_parameters = self.parameters();
                    for (int i = self.numParams()-1; i>=0; i--)
                    {
                        int item_id = nc_parameters.indices()[i];
                        parameters[self.paramName(item_id)] = self.param(item_id).get<double>();
                    }
                    return parameters;
                })

            .def_property_readonly("input_names", [](const syn::Unit& self)
                {
                    std::vector<string> input_names;
                    auto nc_inputs = self.inputs();
                    for (int i = self.numInputs()-1; i>=0; i--)
                    {
                        int item_id = nc_inputs.indices()[i];
                        input_names.push_back(self.inputName(item_id));
                    }
                    return input_names;
                })

            .def("output", [](const syn::Unit& self, const std::string& a_outputName)
                {
                    return self.outputs()[a_outputName];
                })
            .def("output", [](const syn::Unit& self, int a_index)
                {
                    return self.outputs().getByIndex(a_index);
                })
            .def_property_readonly("output_names", [](const syn::Unit& self)
                {
                    std::vector<string> output_names;
                    auto nc_outputs = self.outputs();
                    for (int i = self.numOutputs()-1; i >= 0; i--)
                    {
                        int item_id = nc_outputs.indices()[i];
                        output_names.push_back(self.outputName(item_id));
                    }
                    return output_names;
                })
            .def_property_readonly("outputs", [](const syn::Unit& self)
                {
                    auto nc_outputs = self.outputs();
                    Eigen::Matrix<double, -1, -1, Eigen::RowMajor> outputs(self.numOutputs(), self.getBufferSize());
                    for (int i = self.numOutputs()-1; i >= 0; i--)
                    {
                        int item_id = nc_outputs.indices()[i];
                        for (int j = 0; j < self.getBufferSize(); j++)
                        {
                            outputs(i, j) = self.readOutput(item_id, j);
                        }
                    }
                    return outputs;
                });

    py::class_<syn::Circuit, std::shared_ptr<syn::Circuit>, PyUnit_tpl<syn::Circuit>>(m, "Circuit", py::base<syn::Unit>())
            .def(py::init<const std::string&>())
            .def("getUnit", static_cast<syn::Unit&(syn::Circuit::*)(int)>(&syn::Circuit::getUnit))
            .def("getConnections", &syn::Circuit::getConnections)
            .def("addUnit", [](syn::Circuit& self, syn::Unit& a_unit) { return self.addUnit(&a_unit); })
            .def("addUnit", [](syn::Circuit& self, syn::Unit& a_unit, int a_id) { return self.addUnit(&a_unit, a_id); })
            .def("removeUnit", &syn::Circuit::removeUnit<int>)
            .def("removeUnit", &syn::Circuit::removeUnit<const std::string&>)
            .def("removeUnit", [](syn::Circuit& self, syn::Unit& a_unit) { return self.removeUnit(&a_unit); })
            .def("connectInternal", &syn::Circuit::connectInternal<string>)
            .def("connectInternal", &syn::Circuit::connectInternal<int>)
            .def("disconnectInternal", &syn::Circuit::disconnectInternal<string>)
            .def("disconnectInternal", &syn::Circuit::disconnectInternal<int>)
            .def("getConnections", &syn::Circuit::getConnectionsToInternalInput);

    py::class_<syn::StateVariableFilter, std::shared_ptr<syn::StateVariableFilter>, PyUnit_tpl<syn::StateVariableFilter>>(m, "StateVariableFilter", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::TrapStateVariableFilter, std::shared_ptr<syn::TrapStateVariableFilter>, PyUnit_tpl<syn::TrapStateVariableFilter>>(m, "TrapStateVariableFilter", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::OnePoleLP, std::shared_ptr<syn::OnePoleLP>, PyUnit_tpl<syn::OnePoleLP>>(m, "OnePoleLP", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::LadderFilter, std::shared_ptr<syn::LadderFilter>, PyUnit_tpl<syn::LadderFilter>>(m, "LadderFilter", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::LadderFilterTwo, std::shared_ptr<syn::LadderFilterTwo>, PyUnit_tpl<syn::LadderFilterTwo>>(m, "LadderFilterTwo", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::BasicOscillator, std::shared_ptr<syn::BasicOscillator>, PyUnit_tpl<syn::BasicOscillator>>(m, "BasicOscillator", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::VosimOscillator, std::shared_ptr<syn::VosimOscillator>, PyUnit_tpl<syn::VosimOscillator>>(m, "VosimOscillator", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::FormantOscillator, std::shared_ptr<syn::FormantOscillator>, PyUnit_tpl<syn::FormantOscillator>>(m, "FormantOscillator", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::ADSREnvelope, std::shared_ptr<syn::ADSREnvelope>, PyUnit_tpl<syn::ADSREnvelope>>(m, "ADSREnvelope", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::LFOOscillator, std::shared_ptr<syn::LFOOscillator>, PyUnit_tpl<syn::LFOOscillator>>(m, "LFOOscillator", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::MemoryUnit, std::shared_ptr<syn::MemoryUnit>, PyUnit_tpl<syn::MemoryUnit>>(m, "MemoryUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::VariableMemoryUnit, std::shared_ptr<syn::VariableMemoryUnit>, PyUnit_tpl<syn::VariableMemoryUnit>>(m, "VariableMemoryUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::PanningUnit, std::shared_ptr<syn::PanningUnit>, PyUnit_tpl<syn::PanningUnit>>(m, "PanningUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::FollowerUnit, std::shared_ptr<syn::FollowerUnit>, PyUnit_tpl<syn::FollowerUnit>>(m, "FollowerUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::DCRemoverUnit, std::shared_ptr<syn::DCRemoverUnit>, PyUnit_tpl<syn::DCRemoverUnit>>(m, "DCRemoverUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::ConstantUnit, std::shared_ptr<syn::ConstantUnit>, PyUnit_tpl<syn::ConstantUnit>>(m, "ConstantUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::SummerUnit, std::shared_ptr<syn::SummerUnit>, PyUnit_tpl<syn::SummerUnit>>(m, "SummerUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::GainUnit, std::shared_ptr<syn::GainUnit>, PyUnit_tpl<syn::GainUnit>>(m, "GainUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::LerpUnit, std::shared_ptr<syn::LerpUnit>, PyUnit_tpl<syn::LerpUnit>>(m, "LerpUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::RectifierUnit, std::shared_ptr<syn::RectifierUnit>, PyUnit_tpl<syn::RectifierUnit>>(m, "RectifierUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::PitchToFreqUnit, std::shared_ptr<syn::PitchToFreqUnit>, PyUnit_tpl<syn::PitchToFreqUnit>>(m, "PitchToFreqUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::FreqToPitchUnit, std::shared_ptr<syn::FreqToPitchUnit>, PyUnit_tpl<syn::FreqToPitchUnit>>(m, "FreqToPitchUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::TanhUnit, std::shared_ptr<syn::TanhUnit>, PyUnit_tpl<syn::TanhUnit>>(m, "TanhUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::SwitchUnit, std::shared_ptr<syn::SwitchUnit>, PyUnit_tpl<syn::SwitchUnit>>(m, "SwitchUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::GateUnit, std::shared_ptr<syn::GateUnit>, PyUnit_tpl<syn::GateUnit>>(m, "GateUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::MidiNoteUnit, std::shared_ptr<syn::MidiNoteUnit>, PyUnit_tpl<syn::MidiNoteUnit>>(m, "MidiNoteUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::VelocityUnit, std::shared_ptr<syn::VelocityUnit>, PyUnit_tpl<syn::VelocityUnit>>(m, "VelocityUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::MidiCCUnit, std::shared_ptr<syn::MidiCCUnit>, PyUnit_tpl<syn::MidiCCUnit>>(m, "MidiCCUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::PassthroughUnit, std::shared_ptr<syn::PassthroughUnit>, PyUnit_tpl<syn::PassthroughUnit>>(m, "PassthroughUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());
    py::class_<syn::InputUnit, std::shared_ptr<syn::InputUnit>, PyUnit_tpl<syn::InputUnit>>(m, "InputUnit", py::base<syn::PassthroughUnit>())
            .def(py::init<const std::string&>());
    py::class_<syn::OutputUnit, std::shared_ptr<syn::OutputUnit>, PyUnit_tpl<syn::OutputUnit>>(m, "OutputUnit", py::base<syn::PassthroughUnit>())
            .def(py::init<const std::string&>());


    return m.ptr();
}
