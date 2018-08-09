#include "Tests.h"
#include <vosimlib/Unit.h>
#include <vosimlib/Circuit.h>
#include <vosimlib/DSPMath.h>
#include <vosimlib/units/StateVariableFilter.h>
#include <vosimlib/units/OscillatorUnit.h>
#include <vosimlib/units/MemoryUnit.h>
#include <vosimlib/VoiceManager.h>
#include <vosimlib/common_serial.h>

#include <sstream>
#include <random>
#include <vosimlib/units/MidiUnits.h>
#include <vosimlib/units/ADSREnvelope.h>
#include <vosimlib/units/MathUnits.h>
#include <vosimlib/tables.h>

std::random_device RandomDevice;

TEST_CASE("Check that serializer can load what it saves", "[serialization]") {
    syn::UnitFactory& uf = syn::UnitFactory::instance();
    uf.addUnitPrototype<syn::StateVariableFilter>("Filters", "svf");
    uf.addUnitPrototype<syn::TrapStateVariableFilter>("Filters", "tsvf");
    uf.addUnitPrototype<syn::OnePoleLPUnit>("Filters", "lag");
    uf.addUnitPrototype<syn::LadderFilterA>("Filters", "ladderA");
    uf.addUnitPrototype<syn::LadderFilterB>("Filters", "ladderB");

    uf.addUnitPrototype<syn::BasicOscillatorUnit>("Oscillators", "basic");
    uf.addUnitPrototype<syn::LFOOscillatorUnit>("Modulators", "LFO");

    uf.addUnitPrototype<syn::MemoryUnit>("DSP", "unit delay");
    uf.addUnitPrototype<syn::VariableMemoryUnit>("DSP", "var delay");

    uf.addUnitPrototype<syn::Circuit>("", "circuit");
    uf.addUnitPrototype<syn::InputUnit>("", "in");
    uf.addUnitPrototype<syn::OutputUnit>("", "out");

    SECTION("Serialize") {
        syn::Circuit* circ = new syn::Circuit("main");
        syn::Unit* svfUnit = new syn::StateVariableFilter("svfUnit");
        syn::Unit* oscUnit = new syn::BasicOscillatorUnit("oscUnit");
        circ->addUnit(svfUnit);
        circ->addUnit(oscUnit);
        circ->connectInternal(*oscUnit, 0, *svfUnit, 0);
        std::stringstream ss;
        {
            json j = circ->operator json();
            ss << j;
        }
        string circuit_str1 = ss.str();
        syn::Circuit* readUnit;
        {
            json j; ss >> j;
            syn::Unit* tmpUnit = syn::Unit::fromJSON(j);
            readUnit = dynamic_cast<syn::Circuit*>(tmpUnit);
        }
        REQUIRE(readUnit != nullptr);
        std::stringstream ss2;
        {
            json j = readUnit->operator json();
            ss2 << j;
        }
        string circuit_str2 = ss2.str();
        REQUIRE(circuit_str1 == circuit_str2);
    }
}

TEST_CASE("Check proc graph linearization", "[proc-graph]") {
    SECTION("Circuit linearization") {
        syn::Circuit* circ = new syn::Circuit("main");
        syn::Unit* svfUnit = new syn::StateVariableFilter("svfUnit");
        syn::Unit* oscUnit = new syn::BasicOscillatorUnit("oscUnit");
        syn::Unit* noteUnit = new syn::MidiNoteUnit("noteUnit");
        syn::Unit* envUnit = new syn::ADSREnvelope("envUnit");
        syn::Unit* gainUnit = new syn::GainUnit("gainUnit");
        syn::Unit* outUnit = &circ->getUnit(circ->getOutputUnitId());
        circ->addUnit(svfUnit);
        circ->addUnit(oscUnit);
        circ->addUnit(noteUnit);
        circ->addUnit(envUnit);
        circ->addUnit(gainUnit);
        circ->connectInternal(*svfUnit, 0, *outUnit, 0);
        circ->connectInternal(*oscUnit, 0, *svfUnit, syn::StateVariableFilter::Input::iAudioIn);
        circ->connectInternal(*oscUnit, 0, *gainUnit, 0);
        circ->connectInternal(*oscUnit, 1, *svfUnit, syn::StateVariableFilter::Input::iFcAdd);
        circ->connectInternal(*noteUnit, 0, *oscUnit, syn::TunedOscillatorUnit::Input::iNote);
        circ->connectInternal(*envUnit, 0, *oscUnit, syn::OscillatorUnit::Input::iGainMul);
        auto&& execOrder = circ->execOrder();
        int execOrderSize = std::count_if(execOrder.begin(), execOrder.end(), [](auto&& ptr) { return ptr != nullptr; });
        REQUIRE(execOrderSize == 6);
        REQUIRE(SYN_CONTAINS(execOrder, svfUnit));
        REQUIRE(SYN_CONTAINS(execOrder, oscUnit));
        REQUIRE(SYN_CONTAINS(execOrder, noteUnit));
        REQUIRE(SYN_CONTAINS(execOrder, envUnit));
        REQUIRE(SYN_CONTAINS(execOrder, gainUnit));
        REQUIRE(SYN_CONTAINS(execOrder, outUnit));
        int svfInd = SYN_VEC_FIND(execOrder, svfUnit);
        int oscInd = SYN_VEC_FIND(execOrder, oscUnit);
        int noteInd = SYN_VEC_FIND(execOrder, noteUnit);
        int envInd = SYN_VEC_FIND(execOrder, envUnit);
        int gainInd = SYN_VEC_FIND(execOrder, gainUnit);
        int outInd = SYN_VEC_FIND(execOrder, outUnit);
        REQUIRE(svfInd < outInd);
        REQUIRE(oscInd < svfInd);
        REQUIRE(oscInd < gainInd);
        REQUIRE(noteInd < oscInd);
        REQUIRE(envInd < oscInd);

        REQUIRE(noteUnit->output(0).buf() != envUnit->output(0).buf());
        REQUIRE(noteUnit->output(0).buf() != oscUnit->output(0).buf());
        REQUIRE(noteUnit->output(0).buf() != oscUnit->output(1).buf());
        REQUIRE(envUnit->output(0).buf() != oscUnit->output(0).buf());
        REQUIRE(envUnit->output(0).buf() != oscUnit->output(1).buf());
        REQUIRE(oscUnit->output(0).buf() != oscUnit->output(1).buf());
        REQUIRE(oscUnit->output(0).buf() != svfUnit->output(0).buf());
        REQUIRE(oscUnit->output(1).buf() != svfUnit->output(0).buf());
    }

    SECTION("Linearization") {
        syn::DirectedProcGraph<int> pg;
        pg.connect(0, 1);
        pg.connect(0, 3);

        pg.connect(1, 3);
        pg.connect(1, 3);
        pg.connect(1, 3);

        pg.connect(2, 3);

        std::vector<std::pair<int, syn::DirectedProcGraph<int>::Props>>&& res = pg.linearize();
        std::vector<int> order(res.size());
        std::transform(res.begin(), res.end(), order.begin(), [](auto&& p) { return p.first; });

        REQUIRE(SYN_CONTAINS(order, 0));
        REQUIRE(SYN_CONTAINS(order, 1));
        REQUIRE(SYN_CONTAINS(order, 2));
        REQUIRE(SYN_CONTAINS(order, 3));

        int loc0 = SYN_VEC_FIND(order, 0);
        int loc1 = SYN_VEC_FIND(order, 1);
        int loc2 = SYN_VEC_FIND(order, 2);
        int loc3 = SYN_VEC_FIND(order, 3);
        REQUIRE(loc0 < loc1);
        REQUIRE(loc0 < loc2);
        REQUIRE(loc0 < loc3);
        REQUIRE(loc1 < loc2);
        REQUIRE(loc1 < loc3);
        REQUIRE(loc2 < loc3);

        REQUIRE(res[loc0].second.layer == 2);
        REQUIRE(res[loc1].second.layer == 1);
        REQUIRE(res[loc2].second.layer == 1);
        REQUIRE(res[loc3].second.layer == 0);
    }
}


template<typename Out, typename In>
static double measure_error(std::function<Out(In)> a_baseFunc, std::function<Out(In)> a_testFunc, In a, In b, int N) {
    Out max_error = 0;
    for (int i = 0; i < N; i++) {
        In phase = i * (b - a) / N + a; // lerp from -3 to 3
        Out base_result = a_baseFunc(phase);
        Out test_result = a_testFunc(phase);
        max_error = std::max(max_error, abs(base_result - test_result));
    }
    return max_error;
}
TEST_CASE("Measure error for rational approximation of tanh", "[fast_tanh]") {
    SECTION("fast_tanh_rat") {
        double max_error = measure_error<double, double>([](double in)->double { return tanh(in); }, syn::fast_tanh_rat<double>, -10.0, 10.0, 20e3);
        INFO("Max error: " << max_error);
        REQUIRE(max_error < 1e-3);
    }
}

TEST_CASE("Check that units are ticked correctly", "[Unit]") {
    SECTION("1-sample buffer solo unit") {
        syn::MemoryUnit mu("mu0");
        double input = 1.0;
        mu.connectInput(0, syn::ReadOnlyBuffer<double>{&input});
        mu.tick();
        double out1 = mu.readOutput(0, 0);
        mu.tick();
        double out2 = mu.readOutput(0, 0);
        REQUIRE(out1 == 0.0);
        REQUIRE(out2 == 1.0);
    }

    SECTION("10-sample buffer solo unit") {
        syn::MemoryUnit mu("mu0");
        typedef Eigen::Array<double, -1, -1, Eigen::RowMajor> io_type;
        io_type inputs(1, 10);
        io_type outputs(1, 10);
        for (int i = 0; i < 10; i++) {
            inputs(0, i) = i;
        }
        mu.tick(inputs, outputs);
        REQUIRE(outputs(0, 0) == 0);
        REQUIRE(outputs(0, 1) == 0);
        REQUIRE(outputs(0, 2) == 1);
        REQUIRE(outputs(0, 3) == 2);
        REQUIRE(outputs(0, 4) == 3);
        REQUIRE(outputs(0, 5) == 4);
        REQUIRE(outputs(0, 6) == 5);
        REQUIRE(outputs(0, 7) == 6);
        REQUIRE(outputs(0, 8) == 7);
        REQUIRE(outputs(0, 9) == 8);
    }

    SECTION("10-sample buffer circuit") {
        const int bufSize = 10;
        Eigen::Array<double, 1, bufSize, Eigen::RowMajor> inputs;
        for (int i = 0; i < 10; i++) {
            inputs(0, i) = i;
        }

        syn::Circuit circ;
        circ.setBufferSize(bufSize);
        int circ_svf_id = circ.addUnit(new syn::StateVariableFilter("circ_svf"));
        circ.connectInput(0, syn::ReadOnlyBuffer<double>{&inputs(0, 0)});
        circ.connectInternal(circ.getInputUnitId(), 0, circ_svf_id, 0);
        circ.connectInternal(circ_svf_id, 0, circ.getOutputUnitId(), 0);
        circ.tick();
        Eigen::Array<double, 1, bufSize, Eigen::RowMajor> circ_output;
        std::copy(circ.output(0).buf(), circ.output(0).buf() + bufSize, &circ_output(0, 0));

        syn::StateVariableFilter svf;
        svf.setBufferSize(bufSize);
        REQUIRE(svf.inputName(0) == "in");
        Eigen::Array<double, -1, -1, Eigen::RowMajor> outputs(4, bufSize);
        svf.tick(inputs, outputs);
        for (int i = 0; i<bufSize; i++)
            REQUIRE(outputs(0, i) == circ_output(0, i));        
    }
}

TEST_CASE("Test resampler", "[Resample]") {
    Eigen::Matrix<double, 128, 1> original_table = Eigen::Array<double, 128, 1>::LinSpaced(0, 2 * SYN_PI).sin();

    Eigen::Matrix<double, 128, 1> identical_table;
    Eigen::Matrix<double, 250, 1> upsampled_table;
    Eigen::Matrix<double, 33, 1> downsampled_table;
    
    for (int i = 0; i < identical_table.size(); i++) {
        double phase = i * (1.0 / identical_table.size());
        identical_table(i) = syn::getresampled_single(original_table.data(), original_table.size(), phase, original_table.size(), syn::lut_blimp_table_offline());
    }
    for (int i = 0; i < downsampled_table.size(); i++) {
        double phase = i * (1.0 / downsampled_table.size());
        downsampled_table(i) = syn::getresampled_single(original_table.data(), original_table.size(), phase, downsampled_table.size(), syn::lut_blimp_table_offline());
    }
    for (int i = 0; i < upsampled_table.size(); i++) {
        double phase = i * (1.0 / upsampled_table.size());
        upsampled_table(i) = syn::getresampled_single(original_table.data(), original_table.size(), phase, upsampled_table.size(), syn::lut_blimp_table_offline());
    }

    Eigen::IOFormat listFmt(Eigen::FullPrecision, 0, ", ", ",\n", "", "", "[", "]");
    std::cout << "original=array(" << original_table.format(listFmt) << ")" << std::endl;
    std::cout << "identical=array(" << identical_table.format(listFmt) << ")" << std::endl;
    std::cout << "upsampled=array(" << upsampled_table.format(listFmt) << ")" << std::endl;
    std::cout << "downsampled=array(" << downsampled_table.format(listFmt) << ")" << std::endl;
}

TEST_CASE("Check IntMap use cases", "[IntMap]") {
    syn::IntMap<int, 8> nc;
    std::array<int, 8> nc_vec{};
    nc_vec.fill(0);
    nc.add(0);
    nc.add(2);
    nc.add(4);
    REQUIRE(nc.getByIndex(0) == 0);
    REQUIRE(nc.getByIndex(1) == 2);
    REQUIRE(nc.getByIndex(2) == 4);

    SECTION("Adding and deleting items") {

        nc.removeById(1);
        REQUIRE(nc.getByIndex(0) == 0);
        REQUIRE(nc.getByIndex(1) == 4);
        nc.add(3);
        REQUIRE(nc.getByIndex(0) == 0);
        REQUIRE(nc.getByIndex(1) == 3);
        REQUIRE(nc.getByIndex(2) == 4);
        nc.removeById(0);
        REQUIRE(nc.getByIndex(0) == 3);
        REQUIRE(nc.getByIndex(1) == 4);
        nc.removeById(1);
        REQUIRE(nc.getByIndex(0) == 4);
        nc.removeById(2);
        REQUIRE(nc.empty());
    }

    SECTION("STL compatibility") {
        std::copy(nc.begin(), nc.end(), &nc_vec[0]);
        REQUIRE(nc_vec[0] == 0);
        REQUIRE(nc_vec[1] == 2);
        REQUIRE(nc_vec[2] == 4);
        std::transform(nc.begin(), nc.end(), &nc_vec[0], [](double x) { return x * x; });
        REQUIRE(nc_vec[0] == 0);
        REQUIRE(nc_vec[1] == 4);
        REQUIRE(nc_vec[2] == 16);
    }

    SECTION("Range-based for loop") {
        int i = 0;
        for(auto e : nc) {
            if (i == 0)
                REQUIRE(e == 0);
            else if (i == 1)
                REQUIRE(e == 2);
            else if (i == 2)
                REQUIRE(e == 4);
            i++;
        }
        REQUIRE(i == nc.size());
    }
}
