#include "Unit.h"
#include "Circuit.h"
#include "units/StateVariableFilter.h"
#include "units/Oscillator.h"
#include "units/VosimOscillator.h"
#include "units/ADSREnvelope.h"
#include "tables.h"
#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <string>

namespace py = pybind11;

typedef Eigen::Matrix<double, -1, -1, Eigen::RowMajor> MatrixXdR;
typedef Eigen::Array<double, -1, -1, Eigen::RowMajor> ArrayXXdR;

template <class Base = syn::Unit>
class PyUnit : public Base {
public:
    using Base::Base; // Inherit constructors

    bool isActive() const override {
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

    void reset() override {
        PYBIND11_OVERLOAD_PURE(void, Base, reset, );
    }

    void MSFASTCALL process_() GCCFASTCALL override {
        PYBIND11_OVERLOAD_PURE(void, Base, process_, );
    }

    std::string getClassName() const override {
        PYBIND11_OVERLOAD_PURE(std::string, Base, getClassName, );
    }

    syn::Unit* _clone() const override {
        PYBIND11_OVERLOAD_PURE(syn::Unit*, Base, _clone, );
    }
};


PYBIND11_PLUGIN(pyVOSIMLib) {
    py::module m("pyVOSIMLib", "Python bindings for VOSIMLib");

    py::class_<syn::Unit, PyUnit<syn::Unit>> unit(m, "Unit");
    unit.def(py::init<const std::string &>())
        .def("tick", [](syn::Unit& self, const MatrixXdR& a_inputs) {
                if (a_inputs.rows() > self.numInputs())
                    throw std::runtime_error("Input buffer should have " + std::to_string(self.numOutputs()) + " rows.");
                ArrayXXdR outputs(self.numOutputs(), a_inputs.cols());
                self.tick(a_inputs, outputs);
                return outputs;
            },
            py::return_value_policy::copy)
        .def("reset", &syn::Unit::reset)
        .def_property_readonly("isActive", &syn::Unit::isActive)

        .def_property("fs", &syn::Unit::fs, &syn::Unit::setFs)
        .def_property("tempo", &syn::Unit::tempo, &syn::Unit::setTempo)
        .def_property("bufSize", &syn::Unit::getBufferSize, &syn::Unit::setBufferSize)

        .def_property_readonly("note", &syn::Unit::note)
        .def_property_readonly("isNoteOn", &syn::Unit::isNoteOn)
        .def("noteOn", &syn::Unit::noteOn)
        .def("noteOff", &syn::Unit::noteOff)
        .def_property_readonly("vel", &syn::Unit::velocity)

        .def("set_param", &syn::Unit::setParam<std::string, double>)
        .def("set_param_norm", [](syn::Unit& self, const std::string& a_paramName, double a_value) {
                return self.param(a_paramName).setNorm(a_value);
            })
        .def_property_readonly("params", [](const syn::Unit& self) {
                std::map<std::string, double> parameters;
                auto nc_parameters = self.parameters();
                for (int i = 0; i < self.numParams(); i++) {
                    int item_id = nc_parameters.ids()[i];
                    parameters[self.paramName(item_id)] = self.param(item_id).get<double>();
                }
                return parameters;
            })

        .def_property_readonly("inputNames", [](const syn::Unit& self) {
                std::vector<std::string> input_names;
                auto nc_inputs = self.inputs();
                for (int i = 0; i < self.numInputs(); i++) {
                    int item_id = nc_inputs.ids()[i];
                    input_names.push_back(self.inputName(item_id));
                }
                return input_names;
            })

        .def("output", [](const syn::Unit& self, const std::string& a_outputName) {
                return self.outputs()[a_outputName];
            })
        .def("output", [](const syn::Unit& self, int a_index) {
                return self.outputs().getByIndex(a_index);
            })
        .def_property_readonly("outputNames", [](const syn::Unit& self) {
                std::vector<std::string> output_names;
                auto nc_outputs = self.outputs();
                for (int i = 0; i < self.numOutputs(); i++) {
                    int item_id = nc_outputs.ids()[i];
                    output_names.push_back(self.outputName(item_id));
                }
                return output_names;
            })
        .def_property_readonly("outputs", [](const syn::Unit& self) {
                auto nc_outputs = self.outputs();
                Eigen::Matrix<double, -1, -1, Eigen::RowMajor> outputs(self.numOutputs(), self.getBufferSize());
                for (int i = 0; i < self.numOutputs(); i++) {
                    int item_id = nc_outputs.ids()[i];
                    for (int j = 0; j < self.getBufferSize(); j++) {
                        outputs(i, j) = self.readOutput(item_id, j);
                    }
                }
                return outputs;
            });

    py::class_<syn::Circuit, PyUnit<syn::Circuit>>(m, "Circuit", unit)
            .def(py::init<const std::string&>())
            .def("get", static_cast<syn::Unit&(syn::Circuit::*)(int)>(&syn::Circuit::getUnit))
            .def("conns", &syn::Circuit::getConnections, "Get all connections.")

            .def("add", [](syn::Circuit& self, syn::Unit& a_unit) { return self.addUnit(&a_unit); })
            .def("add", [](syn::Circuit& self, syn::Unit& a_unit, int a_id) { return self.addUnit(&a_unit, a_id); })

            .def("remove", (bool (syn::Circuit::*)(const std::string&))&syn::Circuit::removeUnit)
            .def("remove", (bool (syn::Circuit::*)(const syn::Unit&))&syn::Circuit::removeUnit)
            .def("remove", (bool (syn::Circuit::*)(int))&syn::Circuit::removeUnit)

            .def("connect", (bool (syn::Circuit::*)(int, int, int, int))&syn::Circuit::connectInternal)
            .def("connect", (bool (syn::Circuit::*)(const std::string&, int, const std::string&, int))&syn::Circuit::connectInternal)
            .def("connect", (bool (syn::Circuit::*)(const syn::Unit&, int, const syn::Unit&, int))&syn::Circuit::connectInternal)

            .def("disconnect", (bool (syn::Circuit::*)(int, int, int, int))&syn::Circuit::disconnectInternal)
            .def("disconnect", (bool (syn::Circuit::*)(const std::string&, int, const std::string&, int))&syn::Circuit::disconnectInternal)
            .def("disconnect", (bool (syn::Circuit::*)(int, int, int, int))&syn::Circuit::disconnectInternal)

            .def("conns", &syn::Circuit::getConnectionsToInternalInput, "Get all connections to an internal input.");

    py::class_<syn::StateVariableFilter, PyUnit<syn::StateVariableFilter>> svf(m, "SVF", unit);
    svf.def(py::init<const std::string&>(), "State variable filter");

    py::class_<syn::TrapStateVariableFilter, PyUnit<syn::TrapStateVariableFilter>>(m, "TSVF", svf)
            .def(py::init<const std::string&>(), "State variable filter ('Zero feedback delay' version)");

    py::class_<syn::OnePoleLP, PyUnit<syn::OnePoleLP>>(m, "OnePoleLP", unit)
            .def(py::init<const std::string&>(), "One pole low pass filter ('Zero feedback delay' verion)");

    py::class_<syn::LadderFilter, PyUnit<syn::LadderFilter>>(m, "LdrA", unit)
            .def(py::init<const std::string&>(), "Ladder filter (type A)");

    py::class_<syn::LadderFilterTwo, PyUnit<syn::LadderFilterTwo>>(m, "LdrB", unit)
            .def(py::init<const std::string&>(), "Ladder filter (type B)");


    py::class_<syn::BasicOscillator, PyUnit<syn::BasicOscillator>>(m, "BasicOsc", unit)
            .def(py::init<const std::string&>());

    py::class_<syn::LFOOscillator, PyUnit<syn::LFOOscillator>>(m, "LFO", unit)
            .def(py::init<const std::string&>());

    py::class_<syn::VosimOscillator, PyUnit<syn::VosimOscillator>>(m, "VosimOsc", unit)
            .def(py::init<const std::string&>());

    py::class_<syn::FormantOscillator, PyUnit<syn::FormantOscillator>>(m, "FmtOsc", unit)
            .def(py::init<const std::string&>());

    py::class_<syn::ADSREnvelope, PyUnit<syn::ADSREnvelope>>(m, "ADSREnvelope", unit)
            .def(py::init<const std::string&>());

    py::class_<syn::LookupTable> lut(m, "LookupTable");
    lut.def("__init__",
           [](syn::LookupTable& inst, const Eigen::RowVectorXd& a_table, double a_inputMin = 0, double a_inputMax = 1, bool a_periodic = true) {
               new(&inst) syn::LookupTable(a_table.data(), a_table.cols(), a_inputMin, a_inputMax, a_periodic);
           })
       .def("__getitem__", [](const syn::LookupTable& a_self, py::array_t<int> a_index) {
               auto closure = [a_self](int i) { return a_self.getraw(i); };
               return py::vectorize(closure)(a_index);
           })
       .def("lerp", [](const syn::LookupTable& a_self, py::array_t<double> a_phase) {
               auto closure = [a_self](double p) { return a_self.getlinear(p); };
               return py::vectorize(closure)(a_phase);
           }, "Linear interpolation (0 <= phase <= 1)")
       .def("plerp", [](const syn::LookupTable& a_self, py::array_t<double> a_phase) {
               auto closure = [a_self](double p) { return a_self.getlinear_periodic(p); };
               return py::vectorize(closure)(a_phase);
           }, "Periodic linear interpolation (-inf < phase < inf).")
       .def_property_readonly("size", &syn::LookupTable::size);

    py::class_<syn::BlimpTable> blimp(m, "BlimpTable", lut);
    blimp.def("__init__",
        [](syn::BlimpTable& inst, const Eigen::RowVectorXd& a_table, int a_taps, int a_res) {
            new(&inst) syn::BlimpTable(a_table.data(), a_table.cols(), a_taps, a_res);
        });
    blimp.def_property_readonly("taps", [](const syn::BlimpTable& a_self) { return a_self.m_taps; });
    blimp.def_property_readonly("res", [](const syn::BlimpTable& a_self) { return a_self.m_res; });

    m.def("blimp_offline", &syn::lut_blimp_table_offline, "Offline blimp table.");
    m.def("blimp_online", &syn::lut_blimp_table_online, "Online blimp table.");

    m.def("resample",
        [](const Eigen::RowVectorXd& a_input, double a_newSize, const syn::BlimpTable& a_blimpTable, bool a_normalize = true) {
            Eigen::RowVectorXd outputs((int)ceil(a_newSize));
            syn::resample_table(a_input.data(), a_input.cols(), outputs.data(), a_newSize, a_blimpTable, a_normalize);
            return outputs;
        },
        "Resample a waveform to have the specified period using fractional sinc interpolation/decimation.",
        py::return_value_policy::copy);


    return m.ptr();
}
