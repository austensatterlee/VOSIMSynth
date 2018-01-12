#include "tests.h"
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

TEST_CASE("Detect subnormals in filter processing", "[denormal]") {
    const double fs = 44.1e3;
    const int N = fs*30;
    double input = 1e-3; // Impulse at -60 dB

    SECTION("Ladder (Type A)") {
        syn::LadderFilterA filt;
        filt.setFs(fs);
        filt.setParam(syn::LadderFilterA::pFc, 20000.0);
        filt.setParam(syn::LadderFilterA::pFb, 0.0);
        filt.setParam(syn::LadderFilterA::pDrv, 0.0);
        filt.connectInput(syn::LadderFilterA::iAudioIn, &input);

        vector<double> out(N);
        int numDenormals = 0;
        for (int i = 0; i < N; i++) {
            filt.tick();
            out[i] = filt.readOutput(0, 0);
            if (syn::isDenormal(out[i])) {
                numDenormals++;
            }
            input = 0.0;
        }

        INFO("Denormal count: " << numDenormals << "/" << N << " (" << numDenormals * 1. / N << "%)");
        REQUIRE(numDenormals == 0);
    }

    SECTION("Ladder (Type B)") {
        syn::LadderFilterB filt;
        filt.setFs(fs);
        filt.setParam(syn::LadderFilterB::pFc, 20000.0);
        filt.setParam(syn::LadderFilterB::pFb, 0.0);
        filt.setParam(syn::LadderFilterB::pDrv, 0.0);
        filt.connectInput(syn::LadderFilterB::iAudioIn, &input);

        vector<double> out(N);
        int numDenormals = 0;
        for (int i = 0; i < N; i++) {
            filt.tick();
            out[i] = filt.readOutput(0, 0);
            if (syn::isDenormal(out[i])) {
                numDenormals++;
            }
            input = 0.0;
        }

        INFO("Denormal count: " << numDenormals << "/" << N << " (" << numDenormals * 1. / N << "%)");
        REQUIRE(numDenormals == 0);
    }

    SECTION("SVF") {
        syn::StateVariableFilter filt;
        filt.setFs(fs);
        filt.setParam(syn::StateVariableFilter::pFc, 20000.0);
        filt.setParam(syn::StateVariableFilter::pRes, 0.0);
        filt.connectInput(syn::StateVariableFilter::iAudioIn, &input);

        vector<double> out(N);
        int numDenormals = 0;
        for (int i = 0; i < N; i++) {
            filt.tick();
            out[i] = filt.readOutput(0, 0);
            if (syn::isDenormal(out[i])) {
                numDenormals++;
            }
            input = 0.0;
        }

        INFO("Denormal count: " << numDenormals << "/" << N << " (" << numDenormals * 1. / N << "%)");
        REQUIRE(numDenormals == 0);
    }

    SECTION("TSVF") {
        syn::TrapStateVariableFilter filt;
        filt.setFs(fs);
        filt.setParam(syn::TrapStateVariableFilter::pFc, 20000.0);
        filt.setParam(syn::TrapStateVariableFilter::pRes, 0.0);
        filt.connectInput(syn::TrapStateVariableFilter::iAudioIn, &input);

        vector<double> out(N);
        int numDenormals = 0;
        for (int i = 0; i < N; i++) {
            filt.tick();
            out[i] = filt.readOutput(0, 0);
            if (syn::isDenormal(out[i])) {
                numDenormals++;
            }
            input = 0.0;
        }

        INFO("Denormal count: " << numDenormals << "/" << N << " (" << numDenormals * 1. / N << "%)");
        REQUIRE(numDenormals == 0);
    }
}

TEST_CASE("Check that units are ticked correctly", "[Unit]") {
    SECTION("1-sample buffer solo unit") {
        syn::MemoryUnit mu("mu0");
        double input = 1.0;
        mu.connectInput(0, &input);
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
        circ.connectInput(0, &inputs(0, 0));
        circ.connectInternal(circ.getInputUnitId(), 0, circ_svf_id, 0);
        circ.connectInternal(circ_svf_id, 0, circ.getOutputUnitId(), 0);
        circ.tick();
        Eigen::Array<double, 1, bufSize, Eigen::RowMajor> circ_output;
        std::copy(circ.outputSource(0), circ.outputSource(0) + bufSize, &circ_output(0, 0));

        syn::StateVariableFilter svf;
        svf.setBufferSize(bufSize);
        REQUIRE(svf.inputName(0) == "in");
        Eigen::Array<double, -1, -1, Eigen::RowMajor> outputs(4, bufSize);
        svf.tick(inputs, outputs);
        for (int i = 0; i<bufSize; i++)
            REQUIRE(outputs(0, i) == circ_output(0, i));        
    }
}

TEST_CASE("Circuit", "[Circuit]") {
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
