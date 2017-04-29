#include "Tests.h"

#include <vosimlib/common_serial.h>
#include <Unit.h>
#include <units/StateVariableFilter.h>
#include <units/Oscillator.h>
#include <Circuit.h>
#include <units/MemoryUnit.h>

#include <sstream>
#include <random>
#include <DSPMath.h>
#include "VoiceManager.h"
#include "units/MathUnits.h"

std::random_device RandomDevice;

template <typename T>
T detect_denormals(const std::vector<T>& a_data) {
    auto min_double = std::numeric_limits<double>::min;
    const double epsilon = min_double();
    int ndenormals = 0;
    for (const T& x : a_data) {
        if (std::abs(x) < epsilon)
            ndenormals++;
    }
    return ndenormals;
}

TEST_CASE("Serialization can load what it saves", "[serialization]") {
    syn::UnitFactory& uf = syn::UnitFactory::instance();
    uf.addUnitPrototype<syn::StateVariableFilter>("Filters", "svf");
    uf.addUnitPrototype<syn::TrapStateVariableFilter>("Filters", "tsvf");
    uf.addUnitPrototype<syn::OnePoleLP>("Filters", "lag");
    uf.addUnitPrototype<syn::LadderFilter>("Filters", "ladderA");
    uf.addUnitPrototype<syn::LadderFilterTwo>("Filters", "ladderB");

    uf.addUnitPrototype<syn::BasicOscillator>("Oscillators", "basic");
    uf.addUnitPrototype<syn::LFOOscillator>("Modulators", "LFO");

    uf.addUnitPrototype<syn::MemoryUnit>("DSP", "unit delay");
    uf.addUnitPrototype<syn::VariableMemoryUnit>("DSP", "var delay");

    uf.addUnitPrototype<syn::Circuit>("", "circuit");
    uf.addUnitPrototype<syn::InputUnit>("", "in");
    uf.addUnitPrototype<syn::OutputUnit>("", "out");

    SECTION("Serialize") {
        syn::Circuit* circ = new syn::Circuit("main");
        syn::Unit* svfUnit = new syn::StateVariableFilter("svfUnit");
        syn::Unit* oscUnit = new syn::BasicOscillator("oscUnit");
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
    Out avg_error = 0;
    for (int i = 0; i < N; i++) {
        In phase = i * (b - a) / N + a; // lerp from -3 to 3
        Out base_result = a_baseFunc(phase);
        Out test_result = a_testFunc(phase);
        avg_error += abs(base_result - test_result);
    }
    avg_error *= 1. / N;
    return avg_error;
}

TEST_CASE("Measure error for rational approximations of tanh.", "[fast_tanh]") {
    SECTION("fast_tanh_rat") {
        double avg_error = measure_error<double, double>([](double in)->double { return tanh(in); }, syn::fast_tanh_rat<double>, -3.0, 3.0, 6e3);
        INFO("Avg. error: " << avg_error);
        REQUIRE(avg_error < 1e-2);
    }
    SECTION("fast_tanh_rat2") {
        double avg_error = measure_error<double, double>([](double in)->double { return tanh(in); }, syn::fast_tanh_rat2<double>, -3.0, 3.0, 6e3);
        INFO("Avg. error: " << avg_error);
        REQUIRE(avg_error < 1e-2);
    }
}

TEST_CASE("Detect subnormals in ladder filter processing.", "[LadderFilter]") {
    const int N = 1.440e6;
    const double fs = 48e3;
    syn::LadderFilter ladder("ladder");
    double input = 1e-3; // Impulse at -60 dB
    ladder.setFs(fs);
    ladder.setParam(syn::LadderFilter::pFc, 20000.0);
    ladder.setParam(syn::LadderFilter::pFb, 1.0);
    ladder.setParam(syn::LadderFilter::pDrv, 0.0);
    ladder.connectInput(0, &input);

    vector<double> ladder_out(N);
    for (int i = 0; i < N; i++) {
        ladder.tick();
        ladder_out[i] = ladder.readOutput(0, 0);
        input = 0.0;
    }

    int ndenormals = detect_denormals(ladder_out);
    INFO("Denormal count: " << ndenormals << "/" << N);
    REQUIRE(ndenormals == 0);
}

TEST_CASE("MemoryUnit stateless tick", "[MemoryUnit]") {
    SECTION("Single MemoryUnit tick") {
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

    SECTION("Buffer MemoryUnit tick") {
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
}

TEST_CASE("SVF stateless tick", "[StateVariableFilter]") {
    syn::StateVariableFilter svf("svf0");
    REQUIRE(svf.inputName(0) == "in");
    Eigen::Array<double, -1, -1, Eigen::RowMajor> inputs(1, 10);
    Eigen::Array<double, -1, -1, Eigen::RowMajor> outputs(4, 10);
    for (int i = 0; i < 10; i++) {
        inputs(0, i) = i;
    }
    svf.tick(inputs, outputs);
    REQUIRE(outputs.any());
}

TEST_CASE("IntMap access by index", "[IntMap]") {
    syn::IntMap<int, 8> nc;
    Eigen::Array<double, 1, -1> nc_vec(8);
    nc.add(0);
    nc.add(2);
    nc.add(4);
    REQUIRE(nc.getByIndex(0) == 0);
    REQUIRE(nc.getByIndex(1) == 2);
    REQUIRE(nc.getByIndex(2) == 4);
    nc_vec[0] = nc.getByIndex(0);
    nc_vec[1] = nc.getByIndex(1);
    nc_vec[2] = nc.getByIndex(2);

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

TEST_CASE("GainUnit automatic input port creation & removal", "[GainUnit]") {
    syn::GainUnit g;
    double dummyInputSrc = 0.5;
    const int* inputIds = g.inputs().ids();

    REQUIRE(g.numInputs() == 2);
    REQUIRE(inputIds[0] == 0);
    REQUIRE(inputIds[1] == 1);
    REQUIRE(!g.isConnected(0));
    REQUIRE(!g.isConnected(1));

    SECTION("Connect both initial inputs ports") {
        g.connectInput(0, &dummyInputSrc);

        REQUIRE(g.numInputs() == 2);
        REQUIRE(inputIds[0] == 0);
        REQUIRE(inputIds[1] == 1);
        REQUIRE(g.isConnected(0));
        REQUIRE(!g.isConnected(1));

        g.connectInput(1, &dummyInputSrc);

        REQUIRE(g.numInputs() == 3);
        REQUIRE(inputIds[0] == 0);
        REQUIRE(inputIds[1] == 1);
        REQUIRE(inputIds[2] == 2);
        REQUIRE(g.isConnected(0));
        REQUIRE(g.isConnected(1));
        REQUIRE(!g.isConnected(2));

        SECTION("Disconnect port 0"){

            g.disconnectInput(0);

            REQUIRE(g.numInputs() == 3);
            REQUIRE(inputIds[0] == 0);
            REQUIRE(inputIds[1] == 1);
            REQUIRE(inputIds[2] == 2);
            REQUIRE(!g.isConnected(0));
            REQUIRE(g.isConnected(1));
            REQUIRE(!g.isConnected(2));

            SECTION("Connect port 2"){
                g.connectInput(2, &dummyInputSrc);

                REQUIRE(g.numInputs() == 3);
                REQUIRE(inputIds[0] == 0);
                REQUIRE(inputIds[1] == 1);
                REQUIRE(inputIds[2] == 2);
                REQUIRE(!g.isConnected(0));
                REQUIRE(g.isConnected(1));
                REQUIRE(g.isConnected(2));
            }

            SECTION("Disconnect port 1 then connect port 2") {                
                g.disconnectInput(1);

                REQUIRE(g.numInputs() == 3);
                REQUIRE(inputIds[0] == 0);
                REQUIRE(inputIds[1] == 1);
                REQUIRE(inputIds[2] == 2);
                REQUIRE(!g.isConnected(0));
                REQUIRE(!g.isConnected(1));
                REQUIRE(!g.isConnected(2));

                g.connectInput(2, &dummyInputSrc);
                
                REQUIRE(g.numInputs() == 3);
                REQUIRE(inputIds[0] == 0);
                REQUIRE(inputIds[1] == 1);
                REQUIRE(inputIds[2] == 2);
                REQUIRE(!g.isConnected(0));
                REQUIRE(!g.isConnected(1));
                REQUIRE(g.isConnected(2));
            }
        }
    }
}
