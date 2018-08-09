#include "vosimlib/Unit.h"
#include "vosimlib/Circuit.h"
#include "vosimlib/units/StateVariableFilter.h"
#include "vosimlib/units/OscillatorUnit.h"
#include "vosimlib/units/VosimOscillator.h"
#include "vosimlib/units/ADSREnvelope.h"
#include "vosimlib/tables.h"
#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <string>
#include <vosimlib/units/MathUnits.h>

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

    void onPitchWheelChange_(double a_value) override {
        PYBIND11_OVERLOAD(void, Base, onPitchWheelChange_, a_value);
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

    void process_() override {
        PYBIND11_OVERLOAD_PURE(void, Base, process_, );
    }

    const std::string& getClassName() const override {
        PYBIND11_OVERLOAD_PURE(const std::string&, Base, getClassName, );
    }

    syn::Unit* _clone() const override {
        PYBIND11_OVERLOAD_PURE(syn::Unit*, Base, _clone, );
    }
};

template <class Base>
class PyLUT : public Base {
public:
    using Base::Base;

    auto operator[](py::array_t<int> a_index) const {
        auto closure = [this](int i) {
                    if (0 <= i && i < this->m_size)
                        return (*static_cast<const Base*>(this))[i];
                    else
                        throw std::invalid_argument("Index is out of bounds.");
                };
        return py::vectorize(closure)(a_index);
    }

    auto lerp(py::array_t<double> a_phase) const {
        auto closure = [this](double p) {
                    auto ind = static_cast<const Base*>(this)->index(p);
                    if (ind >= 0 && ind <= this->m_size-1)
                        return static_cast<const Base*>(this)->lerp(p);
                    else
                        throw std::invalid_argument("Phase is out of bounds.");
                };
        return py::vectorize(closure)(a_phase);
    }

    auto plerp(py::array_t<double> a_phase) const {
        auto closure = [this](double p) { return static_cast<const Base*>(this)->plerp(p); };
        return py::vectorize(closure)(a_phase);
    }
};


PYBIND11_PLUGIN(pyVOSIMLib) {
    py::module m("pyVOSIMLib", "Python bindings for VOSIMLib");

    py::class_<syn::Unit, PyUnit<syn::Unit>> unit(m, "Unit");
    unit.def(py::init<const std::string &>())
        .def("tick", [](syn::Unit& self, const MatrixXdR& a_inputs) {
                if (a_inputs.rows() > self.numInputs())
                    throw std::runtime_error("Input buffer should have at most " + std::to_string(self.numOutputs()) + " rows.");
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

        .def("set", &syn::Unit::setParam<std::string, double>)
        .def("setNorm", [](syn::Unit& self, const std::string& a_paramName, double a_value) {
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
                auto ncInputs = self.inputs();
                for (int i = 0; i < self.numInputs(); i++) {
                    int item_id = ncInputs.ids()[i];
                    input_names.push_back(self.inputName(item_id));
                }
                return input_names;
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

            .def("remove", (bool (syn::Circuit::*)(const syn::Unit&))&syn::Circuit::removeUnit)
            .def("remove", (bool (syn::Circuit::*)(int))&syn::Circuit::removeUnit)

            .def("connect", (bool (syn::Circuit::*)(int, int, int, int))&syn::Circuit::connectInternal)
            .def("connect", (bool (syn::Circuit::*)(const syn::Unit&, int, const syn::Unit&, int))&syn::Circuit::connectInternal)

            .def("disconnect", (bool (syn::Circuit::*)(int, int, int, int))&syn::Circuit::disconnectInternal)
            .def("disconnect", (bool (syn::Circuit::*)(int, int, int, int))&syn::Circuit::disconnectInternal)

            .def("conns", &syn::Circuit::getConnectionsToInternalInput, "Get all connections to an internal input.");

    py::class_<syn::StateVariableFilter, PyUnit<syn::StateVariableFilter>> svf(m, "SVF", unit);
    svf.def(py::init<const std::string&>(), "State variable filter");

    py::class_<syn::TrapStateVariableFilter, PyUnit<syn::TrapStateVariableFilter>>(m, "TSVF", svf)
            .def(py::init<const std::string&>(), "State variable filter (TPT version)");

    py::class_<syn::OnePoleLPUnit, PyUnit<syn::OnePoleLPUnit>>(m, "OnePoleLP", unit)
            .def(py::init<const std::string&>(), "One pole low pass filter (TPT verion)");

    py::class_<syn::LadderFilterA, PyUnit<syn::LadderFilterA>>(m, "LdrA", unit)
            .def(py::init<const std::string&>(), "Ladder filter (type A)");

    py::class_<syn::LadderFilterB, PyUnit<syn::LadderFilterB>>(m, "LdrB", unit)
            .def(py::init<const std::string&>(), "Ladder filter (type B)");

    py::class_<syn::BasicOscillatorUnit, PyUnit<syn::BasicOscillatorUnit>>(m, "osc", unit)
            .def(py::init<const std::string&>(), "Basic band-limited waveform oscillator");

    py::class_<syn::LFOOscillatorUnit, PyUnit<syn::LFOOscillatorUnit>>(m, "LFO", unit)
            .def(py::init<const std::string&>());

    py::class_<syn::VosimOscillator, PyUnit<syn::VosimOscillator>>(m, "vosim", unit)
            .def(py::init<const std::string&>(), "VOSIM oscillator");

    py::class_<syn::FormantOscillator, PyUnit<syn::FormantOscillator>>(m, "fmt", unit)
            .def(py::init<const std::string&>(), "Formant oscillator");

    py::class_<syn::ADSREnvelope, PyUnit<syn::ADSREnvelope>>(m, "ADSR", unit)
            .def(py::init<const std::string&>());

    py::class_<syn::DCRemoverUnit, PyUnit<syn::DCRemoverUnit>>(m, "DC", unit)
        .def(py::init<const std::string&>(), "DC killer");

    py::class_<syn::MemoryUnit, PyUnit<syn::MemoryUnit>>(m, "delay1", unit)
        .def(py::init<const std::string&>(), "single sample delay");

    py::class_<syn::VariableMemoryUnit, PyUnit<syn::VariableMemoryUnit>>(m, "delayN", unit)
        .def(py::init<const std::string&>(), "multi-sample fractional delay");


    py::class_<syn::NormalTable, PyLUT<syn::NormalTable>> normalLut(m, "NormalTable");
    normalLut.def("__init__",
                 [](syn::NormalTable& inst, const Eigen::RowVectorXd& a_table) {
                     new(&inst) syn::NormalTable(a_table.data(), a_table.cols());
                 })
             .def("__getitem__", [](const PyLUT<syn::NormalTable>& a_self, py::array_t<int> a_index) {
                     return a_self[a_index];
                 })
             .def("lerp", [](const PyLUT<syn::NormalTable>& a_self, py::array_t<double> a_phase) {
                     return a_self.lerp(a_phase);
                 }, "Linear interpolation (0 <= phase <= 1)")
             .def("plerp", [](const PyLUT<syn::NormalTable>& a_self, py::array_t<double> a_phase) {
                     return a_self.plerp(a_phase);
                 }, "Periodic linear interpolation (-inf < phase < inf).")
             .def_property_readonly("size", [](const syn::NormalTable& a_self) { return a_self.m_size; });

    py::class_<syn::AffineTable, PyLUT<syn::AffineTable>> affineLut(m, "AffineTable");
    affineLut.def("__init__",
                 [](syn::AffineTable& inst, const Eigen::RowVectorXd& a_table, double a_inputMin = 0, double a_inputMax = 1) {
                     new(&inst) syn::AffineTable(a_table.data(), a_table.cols(), a_inputMin, a_inputMax);
                 })
             .def("__getitem__", [](const PyLUT<syn::AffineTable>& a_self, py::array_t<int> a_index) {
                     return a_self[a_index];
                 })
             .def("lerp", [](const PyLUT<syn::AffineTable>& a_self, py::array_t<double> a_phase) {
                     return a_self.lerp(a_phase);
                 }, "Linear interpolation")
             .def("plerp", [](const PyLUT<syn::AffineTable>& a_self, py::array_t<double> a_phase) {
                     return a_self.plerp(a_phase);
                 }, "Periodic linear interpolation (-inf < phase < inf).")
             .def_property_readonly("size", [](const syn::AffineTable& a_self) { return a_self.m_size; });

    py::class_<syn::BlimpTable, PyLUT<syn::BlimpTable>> blimp(m, "BlimpTable", normalLut);
    blimp.def("__init__", [](syn::BlimpTable& inst, const Eigen::RowVectorXd& a_table, int a_taps, int a_res) {
                 new(&inst) syn::BlimpTable(a_table.data(), a_table.cols(), a_taps, a_res);
             })
         .def("__getitem__", [](const PyLUT<syn::BlimpTable>& a_self, py::array_t<int> a_index) {
                 return a_self[a_index];
             })
         .def("lerp", [](const PyLUT<syn::BlimpTable>& a_self, py::array_t<double> a_phase) {
                 return a_self.lerp(a_phase);
             }, "Linear interpolation (0 <= phase <= 1)")
         .def("plerp", [](const PyLUT<syn::BlimpTable>& a_self, py::array_t<double> a_phase) {
                 return a_self.plerp(a_phase);
             }, "Periodic linear interpolation (-inf < phase < inf).")
         .def_property_readonly("taps", [](const syn::BlimpTable& a_self) { return a_self.taps; })
         .def_property_readonly("res", [](const syn::BlimpTable& a_self) { return a_self.res; })
         .def_property_readonly("size", [](const syn::BlimpTable& a_self) { return a_self.m_size; });

    py::class_<syn::ResampledTable, PyLUT<syn::ResampledTable>> rstable(m, "ResampledTable", normalLut);
    rstable.def("__init__",
               [](syn::ResampledTable& inst, const Eigen::RowVectorXd& a_table, const syn::BlimpTable& a_blimp_table_online, const syn::BlimpTable& a_blimp_table_offline ) {
                   new(&inst) syn::ResampledTable(a_table.data(), a_table.cols(), a_blimp_table_online, a_blimp_table_offline);
               })
           .def("__getitem__", [](const PyLUT<syn::ResampledTable>& a_self, py::array_t<int> a_index) {
                   return a_self[a_index];
               })
           .def("lerp", [](const PyLUT<syn::ResampledTable>& a_self, py::array_t<double> a_phase) {
                   return a_self.lerp(a_phase);
               }, "Linear interpolation (0 <= phase <= 1)")
           .def("plerp", [](const PyLUT<syn::ResampledTable>& a_self, py::array_t<double> a_phase) {
                   return a_self.plerp(a_phase);
               }, "Periodic linear interpolation (-inf < phase < inf).")
           .def("get", [](syn::ResampledTable& a_self, py::array_t<double> a_phase, py::array_t<double> a_period) {                   
                    auto closure = [&a_self](double phase, double period)
                    {
                        return a_self.getResampled(phase, period);
                    };
                    return py::vectorize(closure)(a_phase, a_period);
               })
           .def_property_readonly("size", [](const syn::ResampledTable& a_self) { return a_self.m_size; })
           .def_property_readonly("resampled_tables", [](const syn::ResampledTable& a_self) { return a_self.resampledTables(); });

    m.def("blimp_offline", &syn::lut_blimp_table_offline, "Offline blimp table.", pybind11::return_value_policy::reference);
    m.def("blimp_online", &syn::lut_blimp_table_online, "Online blimp table.", pybind11::return_value_policy::reference);
    m.def("bl_saw", &syn::lut_bl_saw_table, "Band-limited saw table.", pybind11::return_value_policy::reference);
    m.def("bl_tri", &syn::lut_bl_tri_table, "Band-limited triangle table.", pybind11::return_value_policy::reference);
    m.def("bl_sqr", &syn::lut_bl_square_table, "Band-limited square table.", pybind11::return_value_policy::reference);
    m.def("pitch_table", &syn::lut_pitch_table, "Pitch table.", pybind11::return_value_policy::reference);
    m.def("sin_table", &syn::lut_sin_table, "Sine table.", pybind11::return_value_policy::reference);

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
