#define NONIUS_RUNNER
#include <nonius/nonius.h++>
#include "bench.h"
#include "vosimlib/tables.h"
#include "vosimlib/DSPMath.h"
#include "vosimlib/IntMap.h"
#include "vosimlib/Unit.h"
#include "vosimlib/Circuit.h"

#include <array>
#include <algorithm>
#include <random>

std::random_device RandomDevice;

NONIUS_BENCHMARK("[lut][sin] lut_sin_table.getraw", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<int> phases(runs);
    const auto& lut_sin_table = syn::lut_sin_table();
    for (int i = 0; i < runs; i++) phases[i] = i * (1.0 / runs) * lut_sin_table.m_size;
    double x;
    meter.measure([&x, &phases, &lut_sin_table](int i) { x = lut_sin_table[phases[i]]; });
})

NONIUS_BENCHMARK("[lut][sin] lut_sin_table.lerp", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    const auto& lut_sin_table = syn::lut_sin_table();
    for (int i = 0; i < runs; i++) phases[i] = i*1.0 / runs;
    double x;
    meter.measure([&x, &phases, &lut_sin_table](int i) { x = lut_sin_table.lerp(phases[i]); });
})

NONIUS_BENCHMARK("[lut][sin] lut_sin_table.plerp", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    const auto& lut_sin_table = syn::lut_sin_table();
    for (int i = 0; i < runs; i++) phases[i] = i*1.0 / runs;
    double x;
    meter.measure([&x, &phases, &lut_sin_table](int i) { x = lut_sin_table.plerp(phases[i]); });
})

NONIUS_BENCHMARK("[lut][sin] std::sin", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = 2*SYN_PI * i * 1.0 / runs;
    double x;
    meter.measure([&x, &phases](int i) { x = std::sin(phases[i]); });
})

NONIUS_BENCHMARK("[math][tan] std::tan", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = phases[i] = syn::LERP(0.0, 20e3 / 44.1e3, i*1.0 / runs);
    double x;
    meter.measure([&x, &phases](int i) { x = std::tan(phases[i]); });
})

NONIUS_BENCHMARK("[math][exp] std::exp", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = phases[i] = syn::LERP(-100.0, 100.0, i*1.0 / runs);
    double x;
    meter.measure([&x, &phases](int i) { x = std::exp(phases[i]); });
})

NONIUS_BENCHMARK("[math][tanh] std::tanh", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = phases[i] = syn::LERP(-10.0, 10.0, i*1.0 / runs);
    double x;
    meter.measure([&x, &phases](int i) { x = std::tanh(phases[i]); });
})

NONIUS_BENCHMARK("[math][tanh] syn::fast_tanh_rat", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = phases[i] = syn::LERP(-10.0, 10.0, i*1.0 / runs);
    double x;
    meter.measure([&x, &phases](int i) { x = syn::fast_tanh_rat<double>(phases[i]); });
})

NONIUS_BENCHMARK("[units][filters] LadderA", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::LadderFilterA ladder("");
    double input = 1.0;
    ladder.setFs(48000.0);
    ladder.setParam(syn::LadderFilterA::pFc, 10000.0);
    ladder.setParam(syn::LadderFilterA::pFb, 0.0);
    ladder.setParam(syn::LadderFilterA::pDrv, 0.0);
    ladder.connectInput(0, syn::ReadOnlyBuffer<double>{ &input });

    double x;
    meter.measure([&x, &ladder](int i)
    {
        ladder.tick();
        x = ladder.readOutput(0,0);
        return x;
    });
})

NONIUS_BENCHMARK("[units][filters] LadderB", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::LadderFilterB ladder("");
    double input = 1.0;
    ladder.setFs(48000.0);
    ladder.setParam(syn::LadderFilterA::pFc, 10000.0);
    ladder.setParam(syn::LadderFilterA::pFb, 0.0);
    ladder.setParam(syn::LadderFilterA::pDrv, 0.0);
    ladder.connectInput(0, syn::ReadOnlyBuffer<double>{ &input });

    double x;
    meter.measure([&x, &ladder](int i)
    {
        ladder.tick();
        x = ladder.readOutput(0,0);
        return x;
    });
})

NONIUS_BENCHMARK("[units][filters] SVF", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::StateVariableFilter svf("");
    double input = 1.0;
    svf.setFs(48000.0);
    svf.setParam(0, 10000.0);
    svf.setParam(1, 0.0);
    svf.connectInput(0, syn::ReadOnlyBuffer<double>{ &input });

    double x;
    meter.measure([&x, &svf](int i)
    {
        svf.tick();
        x = svf.readOutput(0,0);
        return x;
    });
})

NONIUS_BENCHMARK("[units][filters] TSVF", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::TrapStateVariableFilter tsvf("");
    double input = 1.0;
    tsvf.setFs(48000.0);
    tsvf.setParam(0, 10000.0);
    tsvf.setParam(1, 0.0);
    tsvf.connectInput(0, syn::ReadOnlyBuffer<double>{ &input });

    double x;
    meter.measure([&x, &tsvf](int i)
    {
        tsvf.tick();
        x = tsvf.readOutput(0,0);
        return x;
    });
})

NONIUS_BENCHMARK("[units][buffer size] Circuit (Buffer Size: 1)", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::Circuit mycircuit = makeTestCircuit();
    mycircuit.setFs(48000.0);
    mycircuit.setBufferSize(1);
    mycircuit.noteOn(60, 127);

    double x;
    meter.measure([&x, &mycircuit](int i)
    {
        for (int j = 0; j < 200; j++) {
            mycircuit.tick();
            x = mycircuit.readOutput(0, 0);
        }
        return x;
    });
})

NONIUS_BENCHMARK("[units][buffer size] Circuit (Buffer Size: 200)", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::Circuit mycircuit = makeTestCircuit();
    mycircuit.noteOn(60, 127);
    mycircuit.setFs(48000.0);
    mycircuit.setBufferSize(200);

    double x;
    meter.measure([&x, &mycircuit](int i)
    {
        mycircuit.tick();
        x = mycircuit.readOutput(0, 0);
        return x;
    });
})

NONIUS_BENCHMARK("[units][buffer size] syn::VoiceManager (Voices: 8, Buffer Size: 1)", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::VoiceManager vm;
    syn::Circuit mycircuit = makeTestCircuit();    
    vm.setPrototypeCircuit(mycircuit);
    vm.setFs(48e3);
    vm.setMaxVoices(8);
    vm.setBufferSize(200);
    vm.setInternalBufferSize(1);
    // Trigger 8 voices
    vm.noteOn(60, 127);
    vm.noteOn(61, 127);
    vm.noteOn(62, 127);
    vm.noteOn(63, 127);
    vm.noteOn(64, 127);
    vm.noteOn(65, 127);
    vm.noteOn(66, 127);
    vm.noteOn(67, 127);

    std::vector<double> leftIn(200,0), rightIn(200,0);
    std::vector<double> leftOut(200,0), rightOut(200,0);
    meter.measure([&leftIn, &leftOut, &rightIn, &rightOut, &vm](int i)
    {
        vm.tick(&leftIn.front(), &rightIn.front(), &leftOut.front(), &rightOut.front());
    });
})

NONIUS_BENCHMARK("[units][buffer size] syn::VoiceManager (Voices: 8, Buffer Size: 200)", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    
    syn::VoiceManager vm;
    syn::Circuit mycircuit = makeTestCircuit();    
    vm.setPrototypeCircuit(mycircuit);
    vm.setFs(48e3);
    vm.setMaxVoices(8);
    vm.setBufferSize(200);
    vm.setInternalBufferSize(200);
    // Trigger 8 voices
    vm.noteOn(60, 127);
    vm.noteOn(61, 127);
    vm.noteOn(62, 127);
    vm.noteOn(63, 127);
    vm.noteOn(64, 127);
    vm.noteOn(65, 127);
    vm.noteOn(66, 127);
    vm.noteOn(67, 127);

    std::vector<double> leftIn(200, 0), rightIn(200, 0);
    std::vector<double> leftOut(200, 0), rightOut(200, 0);
    meter.measure([&leftIn, &leftOut, &rightIn, &rightOut, &vm](int i)
    {
        vm.tick(&leftIn.front(), &rightIn.front(), &leftOut.front(), &rightOut.front());
    });
})

NONIUS_BENCHMARK("[math][mod] std::fmod", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = syn::LERP(-100.0, 100.0, i*1.0/runs);
    double x;
    meter.measure([&x, &phases](int i)
    {
        x = std::fmod(phases[i], 1.0); 
        return x;
    });
})

NONIUS_BENCHMARK("[math][mod] syn::WRAP", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<double> phases(runs);
    for (int i = 0; i < runs; i++) phases[i] = syn::LERP(-100.0, 100.0, i*1.0 / runs);
    double x;
    meter.measure([&x, &phases](int i)
    {
        x = syn::WRAP(phases[i], 1.0);
        return x;
    });
})

NONIUS_BENCHMARK("[lut][saw] band-limited saw", [](nonius::chronometer& meter) {
    std::vector<double> periods(meter.runs());
    std::vector<double> phases(meter.runs());
    std::uniform_int_distribution<> _periodGenerator(2, syn::lut_bl_saw_table().m_size - 1);
    std::uniform_real_distribution<> _phaseGenerator(0.0, 1.0);
    std::transform(periods.begin(), periods.end(), periods.begin(), [&_periodGenerator](double& x) {return _periodGenerator(RandomDevice); });
    std::transform(phases.begin(), phases.end(), phases.begin(), [&_phaseGenerator](double& x) {return _phaseGenerator(RandomDevice); });
    double x;
    meter.measure([&x, &periods, &phases](int i)
    {
        x = syn::lut_bl_saw_table().getresampled(phases[i], periods[i]);
        return x;
    });
})

NONIUS_BENCHMARK("[lut][saw] naive saw", [](nonius::chronometer& meter) {
    std::vector<double> phases(meter.runs());
    std::uniform_real_distribution<> _phaseGenerator(0.0, 1.0);
    std::transform(phases.begin(), phases.end(), phases.begin(), [&_phaseGenerator](double& x) {return _phaseGenerator(RandomDevice); });
    double x;
    meter.measure([&x, &phases](int i)
    {
        x = syn::naive_saw(phases[i]);
        return x;
    });
})

NONIUS_BENCHMARK("[lut][pitch] lut pitch2freq", [](nonius::chronometer& meter) {
    int runs = meter.runs();
    std::vector<double> pitches(runs);
    for (int i = 0; i < runs; i++) pitches[i] = syn::LERP(-128.0, 128.0, i*1.0 / runs);
    double x;
    meter.measure([&x, &pitches](int i)
    {
        x = syn::pitchToFreq(pitches[i]);
        return x;
    });
})

NONIUS_BENCHMARK("[lut][pitch] naive pitch2freq", [](nonius::chronometer& meter) {
    int runs = meter.runs();
    std::vector<double> pitches(runs);
    for (int i = 0; i < runs; i++) pitches[i] = syn::LERP(-128.0, 128.0, i*1.0 / runs);
    double x;
    meter.measure([&x, &pitches](int i)
    {
        x = syn::naive_pitchToFreq(pitches[i]);
        return x;
    });
})

#define container_size 1024
NONIUS_BENCHMARK("[container] syn::IntMap", [](nonius::chronometer& meter)
{
    std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
    std::vector<int> loads(meter.runs());
    std::vector<int> stores(meter.runs());
    std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
    std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

    syn::IntMap<syn::InputPort<double>, container_size> myContainer;
    for (int i = 0; i < container_size; i++) myContainer.add(syn::InputPort<double>(i));

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})

NONIUS_BENCHMARK("[container] std::unordered_map<int>", [](nonius::chronometer& meter)
{
    std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
    std::vector<int> loads(meter.runs());
    std::vector<int> stores(meter.runs());
    std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
    std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

    std::unordered_map<int, syn::InputPort<double>> myContainer;
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort<double>(i);

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})

NONIUS_BENCHMARK("[container] std::array", [](nonius::chronometer& meter)
{
    std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
    std::vector<int> loads(meter.runs());
    std::vector<int> stores(meter.runs());
    std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
    std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

    std::array<syn::InputPort<double>, container_size> myContainer;
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort<double>(i);

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})

NONIUS_BENCHMARK("[container] std::vector", [](nonius::chronometer& meter)
{
    std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
    std::vector<int> loads(meter.runs());
    std::vector<int> stores(meter.runs());
    std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
    std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

    std::vector<syn::InputPort<double>> myContainer(container_size);
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort<double>(i);

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})

NONIUS_BENCHMARK("[container] std::map<int>", [](nonius::chronometer& meter)
{
    std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
    std::vector<int> loads(meter.runs());
    std::vector<int> stores(meter.runs());
    std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
    std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

    std::map<int, syn::InputPort<double>> myContainer;
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort<double>(i);

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})

