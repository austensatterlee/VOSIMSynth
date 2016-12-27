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

namespace py = pybind11;

template <class Base = syn::Unit>
class PyUnit_tpl : public Base
{
public:
    using Base::Base; // Inherit constructors
    void reset() override {
        PYBIND11_OVERLOAD(void, Base, reset, );
    }

    bool isActive() override {
        PYBIND11_OVERLOAD(bool, Base, isActive, );
    }

    void onParamChange_(int a_paramId) override {
        PYBIND11_OVERLOAD(void, Base, onParamChange_, a_paramId);
    };

    void onFsChange_() override {
        PYBIND11_OVERLOAD(void, Base, onFsChange_, );
    }

    void onTempoChange_() override {
        PYBIND11_OVERLOAD(void, Base, onTempoChange_, );
    }

    void onNoteOn_() override {
        PYBIND11_OVERLOAD(void, Base, onNoteOn_, );
    }

    void onNoteOff_() override {
        PYBIND11_OVERLOAD(void, Base, onNoteOff_, );
    }

    void onMidiControlChange_(int a_cc, double a_value) override {
        PYBIND11_OVERLOAD(void, Base, onMidiControlChange_, a_cc, a_value);
    }

    void onInputConnection_(int a_inputPort) override {
        PYBIND11_OVERLOAD(void, Base, onInputConnection_, a_inputPort);
    }

    void onInputDisconnection_(int a_inputPort) override {
        PYBIND11_OVERLOAD(void, Base, onInputDisconnection_, a_inputPort);
    }

    void MSFASTCALL process_() GCCFASTCALL override {
        PYBIND11_OVERLOAD_PURE(void, Base, process_, );
    }

    inline string _getClassName() const override {
        PYBIND11_OVERLOAD_PURE(std::string, Base, _getClassName, );
    }

    syn::Unit *_clone() const override {
        PYBIND11_OVERLOAD_PURE(syn::Unit*, Base, _clone, );
    }
};

PYBIND11_PLUGIN(pyVOSIMLib) {
    py::module m("pyVOSIMLib", "Python bindings for VOSIMLib");

    py::class_<syn::Unit, std::unique_ptr<syn::Unit>,
               PyUnit_tpl<syn::Unit>>(m, "Unit")
            .def(py::init<const std::string &>())
            .def("tick", [](syn::Unit& self, const Eigen::Array<double, -1, -1, Eigen::RowMajor>& a_inputs) {
                     if (a_inputs.rows() > self.numInputs())
                         throw std::runtime_error("Incompatible buffer format!");
                     Eigen::Array<double, -1, -1, Eigen::RowMajor> outputs(self.numOutputs(), a_inputs.cols());
                     self.tick(a_inputs, outputs);
                     return outputs;
                 }
            )
            .def("tick", static_cast<void (syn::Unit::*)()>(&syn::Unit::tick))
            .def("reset", &syn::Unit::reset)
            .def("isActive", &syn::Unit::isActive)
            .def("setFs", &syn::Unit::setFs)
            .def("setTempo", &syn::Unit::setTempo)
            .def("noteOn", &syn::Unit::noteOn)
            .def("noteOff", &syn::Unit::noteOff)
            .def("notifyParameterChanged", &syn::Unit::notifyParameterChanged)
            .def("fs", &syn::Unit::fs)
            .def("tempo", &syn::Unit::tempo)
            .def("isNoteOn", &syn::Unit::isNoteOn)
            .def("note", &syn::Unit::note)
            .def("velocity", &syn::Unit::velocity)

            .def("setParam", &syn::Unit::setParam<string,double>)
            .def_property_readonly("parameters", [](const syn::Unit& self) {
                                       std::map<string, double> parameters;
                                       auto nc_parameters = self.parameters();
                                       for (int i = 0; i < self.numParams(); i++) {
                                           int item_id = nc_parameters.indices()[i];
                                           parameters[self.paramName(item_id)] = self.param(item_id).get<double>();
                                       }
                                       return parameters;
                                   })


            .def_property_readonly("input_names", [] (const syn::Unit& self) {
                                       std::vector<string> input_names;
                                       auto nc_inputs = self.inputs();
                                       for (int i = 0; i < self.numInputs(); i++) {
                                           int item_id = nc_inputs.indices()[i];
                                           input_names.push_back(self.inputName(item_id));
                                       }
                                       return input_names;
                                   })

            .def("output", &syn::Unit::readOutput<string>)
            .def("output", [](const syn::Unit& self, int a_index) { return self.outputs().getByIndex(a_index); })
            .def_property_readonly("output_names", [](const syn::Unit& self) {
                                       std::vector<string> output_names;
                                       auto nc_outputs = self.outputs();
                                       for (int i = 0; i < nc_outputs.size(); i++) {
                                           int item_id = nc_outputs.indices()[i];
                                           output_names.push_back(self.outputName(item_id));
                                       }
                                       return output_names;
                                   })
            .def_property_readonly("outputs", [](const syn::Unit& self) {
                                       std::vector<double> outputs;
                                       auto nc_outputs = self.outputs();
                                       for (int i = 0; i < nc_outputs.size(); i++) {
                                           int item_id = nc_outputs.indices()[i];
                                           outputs.push_back(self.readOutput(item_id));
                                       }
                                       return outputs;
                                   });

    py::class_<syn::Circuit, std::unique_ptr<syn::Circuit>, PyUnit_tpl<syn::Circuit>>(m, "Circuit", py::base<syn::Unit>())
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

    py::class_<syn::StateVariableFilter, std::unique_ptr<syn::StateVariableFilter>, PyUnit_tpl<syn::StateVariableFilter>>(m, "StateVariableFilter", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::TrapStateVariableFilter, std::unique_ptr<syn::TrapStateVariableFilter>, PyUnit_tpl<syn::TrapStateVariableFilter>>(m, "TrapStateVariableFilter", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::LadderFilter, std::unique_ptr<syn::LadderFilter>, PyUnit_tpl<syn::LadderFilter>>(m, "LadderFilterA", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::LadderFilterTwo, std::unique_ptr<syn::LadderFilterTwo>, PyUnit_tpl<syn::LadderFilterTwo>>(m, "LadderFilterB", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::OnePoleLP, std::unique_ptr<syn::OnePoleLP>, PyUnit_tpl<syn::OnePoleLP>>(m, "OnePoleLP", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::BasicOscillator, std::unique_ptr<syn::BasicOscillator>, PyUnit_tpl<syn::BasicOscillator>>(m, "BasicOscillator", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::LFOOscillator, std::unique_ptr<syn::LFOOscillator>, PyUnit_tpl<syn::LFOOscillator>>(m, "LFOOscillator", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::VosimOscillator, std::unique_ptr<syn::VosimOscillator>, PyUnit_tpl<syn::VosimOscillator>>(m, "VosimOscillator", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::FormantOscillator, std::unique_ptr<syn::FormantOscillator>, PyUnit_tpl<syn::FormantOscillator>>(m, "FormantOscillator", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::ADSREnvelope, std::unique_ptr<syn::ADSREnvelope>, PyUnit_tpl<syn::ADSREnvelope>>(m, "ADSREnvelope", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    py::class_<syn::VariableMemoryUnit, std::unique_ptr<syn::VariableMemoryUnit>, PyUnit_tpl<syn::VariableMemoryUnit>>(m, "ResampleUnit", py::base<syn::Unit>())
            .def(py::init<const std::string&>());

    return m.ptr();
}
