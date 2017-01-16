#include "Tests.h"

#include <common_serial.h>
#include <Unit.h>
#include <StateVariableFilter.h>
#include <Oscillator.h>
#include <Circuit.h>
#include <MemoryUnit.h>

#include <sstream>
#include <memory>
#include <random>
#include <cmath>
#include <DSPMath.h>
#include "VoiceManager.h"
#include "MidiUnits.h"
#include "ThreadPool.h"
#include "ADSREnvelope.h"

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

TEST_CASE("Serialization example.", "[serialization]") {
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
        circ->connectInternal("oscUnit", 0, "svfUnit", 0);
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
    Out base_result, test_result;
    Out avg_error = 0;
    In phase;
    for (int i = 0; i < N; i++) {
        phase = i * (b-a) / N + a; // lerp from -3 to 3
        base_result = a_baseFunc(phase);
        test_result = a_testFunc(phase);
        avg_error += abs(base_result - test_result);
    }
    avg_error *= 1. / N;
    return avg_error;
}

TEST_CASE("Measure error for rational approximations of tanh.",    "[fast-tanh]") {
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

TEST_CASE("Detect subnormals in ladder filter processing.",    "[ladder-denormal]") {
    const int N = 2.88e6;
    const double fs = 48e3;
    syn::LadderFilter ladder("ladder");
    double input = 1.0;
    ladder.setFs(fs);
    ladder.setParam(syn::LadderFilter::pFc, 1.0);
    ladder.setParam(syn::LadderFilter::pFb, 1.0);
    ladder.setParam(syn::LadderFilter::pDrv, 1.0);
    ladder.connectInput(0, &input);

    vector<double> ladder_out(N);
    for (int i = 0; i < N; i++) {
        ladder.tick();
        ladder_out[i] = ladder.readOutput(0,0);
        input = 0.0;
    }

    int ndenormals = detect_denormals(ladder_out);
    INFO( "Denormal count: " << ndenormals << "/" << N);
    REQUIRE(ndenormals == 0);
}

TEST_CASE("Test stateless MemoryUnit::tick", "[unit]") {
    SECTION("Single MemoryUnit tick") {
        syn::MemoryUnit mu("mu0");
        double input = 1.0;
        mu.connectInput(0, &input);
        mu.tick();
        double out1 = mu.readOutput(0,0);
        mu.tick();
        double out2 = mu.readOutput(0,0);
        REQUIRE(out1 == 0.0);
        REQUIRE(out2 == 1.0);
    }

    SECTION("Buffer MemoryUnit tick") {
        syn::MemoryUnit mu("mu0");
        typedef Eigen::Array<double, -1, -1, Eigen::RowMajor> io_type;
        io_type inputs(1, 10);
        io_type outputs(1, 10);
        for (int i = 0; i<10; i++) {
            inputs(0, i) = i;
        }
        INFO("Inputs: " << inputs);
        mu.tick(inputs, outputs);
        INFO("Outputs: " << outputs);
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


TEST_CASE("Test stateless StateVariableFilter::tick", "[unit]") {
    syn::StateVariableFilter svf("svf0");
    Eigen::Array<double, -1, -1, Eigen::RowMajor> inputs(1, 10);
    Eigen::Array<double, -1, -1, Eigen::RowMajor> outputs(4, 10);
    for (int i = 0; i<10; i++) {
        inputs(0, i) = i;
    }
    INFO("Inputs: " << inputs);
    svf.tick(inputs, outputs);
    INFO("Outputs: " << outputs);
    REQUIRE(outputs.any());
}

TEST_CASE("Test thread pool", "[threading]")
{
    int nSamples =  10000;
    int bufSize = 64;
    std::array<syn::Circuit*, 8> circuits;
    std::array<syn::CircuitThreadJob*, 8> workItems;
    for(int i=0;i<circuits.size();i++)
    {
        circuits[i] = new syn::Circuit("");
        syn::Unit* gateUnit = new syn::GateUnit("gateUnit");
        syn::Unit* envUnit = new syn::ADSREnvelope("envUnit");
        envUnit->param(syn::ADSREnvelope::Param::pDecay).setNorm(1.0);
        envUnit->param(syn::ADSREnvelope::Param::pSustain).setNorm(0.0);
        circuits[i]->addUnit(gateUnit);
        circuits[i]->addUnit(envUnit);
        circuits[i]->connectInternal<std::string>("gateUnit", 0, "envUnit", 0);
        circuits[i]->connectInternal<std::string>("envUnit", 0, "outputs", 0);
        circuits[i]->setBufferSize(bufSize);
        circuits[i]->noteOn(9, 255);
        circuits[i]->setFs(nSamples);
        workItems[i] = new syn::CircuitThreadJob;
        workItems[i]->circuit = circuits[i];
    }

    Eigen::Matrix<double,1,-1,Eigen::RowMajor> base_output(nSamples);
    base_output.fill(0.0);
    for(int i=0;i<nSamples;i+=bufSize)
    {
        for(int j=0;j<circuits.size();j++)
        {
            circuits[j]->tick();
            for(int k=0;k<bufSize && i+k<nSamples;k++)
            {
                base_output(i+k) += circuits[j]->readOutput(0,k);
            }
        }
    }  

    for(syn::Circuit* circuit : circuits)
    {
        circuit->reset();
    }

    SECTION("0 threads"){
        syn::ThreadPool pool(1024);
        pool.resizePool(0);
        Eigen::Matrix<double,1,-1,Eigen::RowMajor> test_output(nSamples);
        test_output.fill(0.0);
        for(int i=0;i<nSamples;i+=bufSize)
        {
            for(int j=0;j<workItems.size();j++)
            {
                pool.push(workItems[j]);
            }
            pool.waitForWorkers();
            for(int j=0;j<workItems.size();j++)
            {
                for(int k=0;k<bufSize && i+k<nSamples;k++)
                {
                    test_output(i+k) += circuits[j]->readOutput(0,k);
                }
            }
        }
        REQUIRE( (test_output.array() == base_output.array()).all() );
    }
    
    SECTION("1 thread"){
        syn::ThreadPool pool(1024);
        pool.resizePool(1);
        Eigen::Matrix<double,1,-1,Eigen::RowMajor> test_output(nSamples);
        test_output.fill(0.0);
        for(int i=0;i<nSamples;i+=bufSize)
        {
            for(int j=0;j<workItems.size();j++)
            {
                pool.push(workItems[j]);
            }
            pool.waitForWorkers();
            for(int j=0;j<workItems.size();j++)
            {
                for(int k=0;k<bufSize && i+k<nSamples;k++)
                {
                    test_output(i+k) += circuits[j]->readOutput(0,k);
                }
            }
        }
        REQUIRE( (test_output.array() == base_output.array()).all() );
    }

    SECTION("2 threads"){
        syn::ThreadPool pool(1024);
        pool.resizePool(2);
        Eigen::Matrix<double,1,-1,Eigen::RowMajor> test_output(nSamples);
        test_output.fill(0.0);
        for(int i=0;i<nSamples;i+=bufSize)
        {
            for(int j=0;j<workItems.size();j++)
            {
                pool.push(workItems[j]);
            }
            pool.waitForWorkers();
            for(int j=0;j<workItems.size();j++)
            {
                for(int k=0;k<bufSize && i+k<nSamples;k++)
                {
                    test_output(i+k) += circuits[j]->readOutput(0,k);
                }
            }
        }
        REQUIRE( (test_output.array() == base_output.array()).all() );
    }
    
    SECTION("4 threads"){
        syn::ThreadPool pool(1024);
        pool.resizePool(4);
        Eigen::Matrix<double,1,-1,Eigen::RowMajor> test_output(nSamples);
        test_output.fill(0.0);
        for(int i=0;i<nSamples;i+=bufSize)
        {
            for(int j=0;j<workItems.size();j++)
            {
                pool.push(workItems[j]);
            }
            pool.waitForWorkers();
            for(int j=0;j<workItems.size();j++)
            {
                for(int k=0;k<bufSize && i+k<nSamples;k++)
                {
                    test_output(i+k) += circuits[j]->readOutput(0,k);
                }
            }
        }
        REQUIRE( (test_output.array() == base_output.array()).all() );
    }
}
