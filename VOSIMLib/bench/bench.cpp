#define NONIUS_RUNNER
#include <nonius/nonius.h++>
#include "vosimlib/tables.h"
#include "vosimlib/DSPMath.h"
#include "vosimlib/IntMap.h"
#include "vosimlib/Unit.h"
#include "vosimlib/Circuit.h"
#include "vosimlib/units/StateVariableFilter.h"
#include "vosimlib/units/OscillatorUnit.h"
#include "vosimlib/units/MidiUnits.h"

#include <array>
#include <algorithm>
#include <random>
#include <string>

std::random_device RandomDevice;

NONIUS_BENCHMARK("[lut][sin] lut_sin_table.getraw", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    std::vector<int> phases(runs);
    const auto& lut_sin_table = syn::lut_sin_table();
    for (int i = 0; i < runs; i++) phases[i] = i * (1.0 / runs) * lut_sin_table.size;
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
    ladder.connectInput(0, &input);

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
    ladder.connectInput(0, &input);

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
    svf.connectInput(0, &input);

    double x;
    meter.measure([&x, &svf](int i)
    {
        svf.tick();
        x = svf.readOutput(0,0);
        return x;
    });
})

NONIUS_BENCHMARK("[units][filters]  TSVF", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    syn::TrapStateVariableFilter tsvf("");
    double input = 1.0;
    tsvf.setFs(48000.0);
    tsvf.setParam(0, 10000.0);
    tsvf.setParam(1, 0.0);
    tsvf.connectInput(0, &input);

    double x;
    meter.measure([&x, &tsvf](int i)
    {
        tsvf.tick();
        x = tsvf.readOutput(0,0);
        return x;
    });
})

NONIUS_BENCHMARK("[units][buffer size] Ladder Circuit (Buffer Size: 1)", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    const int nUnits = 12;
    syn::Circuit mycircuit("main");
    mycircuit.setFs(48000.0);
    mycircuit.setBufferSize(1);
    // Add a bunch of ladder filter units in serial
    syn::LadderFilterA* lf[nUnits];
    lf[0] = new syn::LadderFilterA("lf0");
    mycircuit.addUnit(lf[0]);
    for (int i = 1; i < nUnits; i++) {
        lf[i] = new syn::LadderFilterA("lf" + std::to_string(i));
        mycircuit.addUnit(lf[i]);
        mycircuit.connectInternal(mycircuit.getUnitId(*lf[i - 1]), 0, mycircuit.getUnitId(*lf[i]), 0);
    }
    // Add pitch and oscillator units
    syn::MidiNoteUnit* mnu = new syn::MidiNoteUnit("mnu0");
    mycircuit.addUnit(mnu);
    syn::BasicOscillatorUnit* bosc = new syn::BasicOscillatorUnit("bosc0");
    mycircuit.addUnit(bosc);
    mycircuit.connectInternal(mycircuit.getUnitId(*mnu), 0, mycircuit.getUnitId(*bosc), 0);
    mycircuit.connectInternal(mycircuit.getUnitId(*bosc), 0, mycircuit.getUnitId(*lf[0]), 0);
    // Connect final ladder unit to circuit output
    mycircuit.connectInternal(mycircuit.getUnitId(*lf[nUnits - 1]), 0, mycircuit.getOutputUnitId(), 0);
    mycircuit.noteOn(60, 127);

    double x;
    meter.measure([&x, &mycircuit](int i)
    {
        for (int i = 0; i < 200; i++) {
            mycircuit.tick();
            x = mycircuit.readOutput(0, 0);
        }
        return x;
    });
})

NONIUS_BENCHMARK("[units][buffer size] Ladder Circuit (Buffer Size: 200)", [](nonius::chronometer& meter) {
    const int runs = meter.runs();
    const int nUnits = 12;
    syn::Circuit mycircuit("main");
    mycircuit.setFs(48000.0);
    mycircuit.setBufferSize(200);
    // Add a bunch of ladder filter units in serial
    syn::LadderFilterA* lf[nUnits];
    lf[0] = new syn::LadderFilterA("lf0");
    mycircuit.addUnit(lf[0]);
    for (int i = 1; i < nUnits; i++) {
        lf[i] = new syn::LadderFilterA("lf" + std::to_string(i));
        mycircuit.addUnit(lf[i]);
        mycircuit.connectInternal(mycircuit.getUnitId(*lf[i - 1]), 0, mycircuit.getUnitId(*lf[i]), 0);
    }
    // Add pitch and oscillator units
    syn::MidiNoteUnit* mnu = new syn::MidiNoteUnit("mnu0");
    mycircuit.addUnit(mnu);
    syn::BasicOscillatorUnit* bosc = new syn::BasicOscillatorUnit("bosc0");
    mycircuit.addUnit(bosc);
    mycircuit.connectInternal(mycircuit.getUnitId(*mnu), 0, mycircuit.getUnitId(*bosc), 0);
    mycircuit.connectInternal(mycircuit.getUnitId(*bosc), 0, mycircuit.getUnitId(*lf[0]), 0);
    // Connect final ladder unit to circuit output
    mycircuit.connectInternal(mycircuit.getUnitId(*lf[nUnits - 1]), 0, mycircuit.getOutputUnitId(), 0);
    mycircuit.noteOn(60, 127);

    double x;
    meter.measure([&x, &mycircuit](int i)
    {
        mycircuit.tick();
        x = mycircuit.readOutput(0, 0);
        return x;
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
    std::uniform_int_distribution<> _periodGenerator(2, syn::lut_bl_saw_table().size - 1);
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

    syn::IntMap<syn::InputPort, container_size> myContainer;
    for (int i = 0; i < container_size; i++) myContainer.add(syn::InputPort(i));

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

    std::unordered_map<int, syn::InputPort> myContainer;
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort(i);

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

    std::array<syn::InputPort, container_size> myContainer;
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort(i);

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

    std::vector<syn::InputPort> myContainer(container_size);
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort(i);

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

    std::map<int, syn::InputPort> myContainer;
    for (int i = 0; i < container_size; i++) myContainer[i] = syn::InputPort(i);

    meter.measure([&myContainer, &stores, &loads](int i)
    {
        myContainer[stores[i]] = myContainer[loads[i]];
    });
})
