#define NONIUS_RUNNER
#include <nonius.h++>

#include "tables.h"
#include "DSPMath.h"
#include "NamedContainer.h"
#include "Unit.h"

#include <array>
#include <algorithm>
#include <random>
#include <string>
#include <StateVariableFilter.h>

#define trig_benches 1
#define ladder_benches 1
#define modulus_benches 1
#define lut_saw_benches 1
#define lut_pitch_benches 1
#define container_benches 1

std::random_device RandomDevice;

#if trig_benches
NONIUS_BENCHMARK("syn::lut_sin.getraw", [](nonius::chronometer& meter) {
	const int runs = meter.runs();
	std::vector<int> phases(runs);
	const syn::LookupTable& lut_sin_table = syn::lut_sin_table();
	auto _phaseGenerator = [&runs, &lut_sin_table](int& n)->int { return n*1.0 / runs*lut_sin_table.size(); };
	std::transform(phases.begin(), phases.end(), phases.begin(), _phaseGenerator);
	double x;
	meter.measure([&x, &phases, &lut_sin_table](int i) { x = lut_sin_table.getraw(phases[i]); });
})

NONIUS_BENCHMARK("syn::lut_sin.getlinear", [](nonius::chronometer& meter) {
	const int runs = meter.runs();
	std::vector<double> phases(runs);
	const syn::LookupTable& lut_sin_table = syn::lut_sin_table();
	for(int i=0;i<runs;i++) phases[i]=i*1.0/runs*lut_sin_table.size();
	double x;
	meter.measure([&x, &phases, &lut_sin_table](int i) { x = lut_sin_table.getlinear(phases[i]); });
})

NONIUS_BENCHMARK("std::sin", [](nonius::chronometer& meter) {
	const int runs = meter.runs();
	std::vector<double> phases(runs);
	const syn::LookupTable& lut_sin_table = syn::lut_sin_table();
	for (int i = 0; i<runs; i++) phases[i] = DSP_PI * i * 1.0 / runs;
	double x;
	meter.measure([&x, &phases](int i) { x = std::sin(phases[i]); });
})

NONIUS_BENCHMARK("std::tanh", [](nonius::chronometer& meter) {
	const int runs = meter.runs();
	std::vector<double> phases(runs);
	for (int i = 0; i<runs; i++) phases[i] = i * 20.0 / runs - 10.;
	double x;
	meter.measure([&x, &phases](int i) { x = std::tanh(phases[i]); });
})

NONIUS_BENCHMARK("syn::fast_tanh::poly", [](nonius::chronometer& meter) {
	const int runs = meter.runs();
	std::vector<double> phases(runs);
	for (int i = 0; i<runs; i++) phases[i] = i * 20.0 / runs - 10.;
	double x;
	meter.measure([&x, &phases](int i) { x = syn::fast_tanh_poly<double>(phases[i]); });
})
#endif

#if ladder_benches
NONIUS_BENCHMARK("ladder", [](nonius::chronometer& meter) {
	const int runs = meter.runs();
	std::vector<double> phases(runs);
	syn::LadderFilter ladder("ladder");
	double input = 1.0;
	ladder.setParameterValue(syn::LadderFilter::pFc, 10000.0);
	ladder.connectInput(0, &input);

	for (int i = 0; i<runs; i++) phases[i] = i * 20.0 / runs - 10.;
	double x;
	meter.measure([&x, &input, &ladder](int i)
	{
		input = 1.0;
		for (int j = 0; j < 48000; j++) {
			ladder.tick();
			x = ladder.getOutputValue(0);
		}
	});
})

NONIUS_BENCHMARK("ladder", [](nonius::chronometer& meter) {
	const int runs = meter.runs();
	std::vector<double> phases(runs);
	syn::LadderFilter ladder("ladder");
	double input = 1.0;
	ladder.setParameterValue(syn::LadderFilter::pFc, 10000.0);
	ladder.connectInput(0, &input);

	for (int i = 0; i<runs; i++) phases[i] = i * 20.0 / runs - 10.;
	double x;
	meter.measure([&x, &input, &ladder](int i)
	{
		input = 1.0;
		for (int j = 0; j < 48000; j++) {
			ladder.tick();
			x = ladder.getOutputValue(0);
		}
	});
})
#endif

#if modulus_benches
NONIUS_BENCHMARK("std::mod", [](nonius::chronometer& meter) {
	const int runs = meter.runs();
	std::vector<double> phases(runs);
	for (int i = 0; i<runs; i++) phases[i] = i * 20.0 / runs - 10.;
	double x;
	meter.measure([&x, &phases](int i) { x = std::fmod(phases[i], 1); });
})

NONIUS_BENCHMARK("syn::WRAP", [](nonius::chronometer& meter) {
	const int runs = meter.runs();
	std::vector<double> phases(runs);
	for (int i = 0; i<runs; i++) phases[i] = i * 20.0 / runs - 10.;
	double x;
	meter.measure([&x, &phases](int i) { x = syn::WRAP(phases[i],1.0); });
})
#endif

#if lut_saw_benches
NONIUS_BENCHMARK("lut saw", [](nonius::chronometer& meter) {
	std::vector<double> periods(meter.runs());
	std::vector<double> phases(meter.runs());
	std::uniform_int_distribution<> _periodGenerator(2, syn::lut_bl_saw_table().size() - 1);
	std::uniform_real_distribution<> _phaseGenerator(0.0,1.0);
	std::transform(periods.begin(), periods.end(), periods.begin(), [&_periodGenerator](double& x) {return _periodGenerator(RandomDevice); });
	std::transform(phases.begin(), phases.end(), phases.begin(), [&_phaseGenerator](double& x) {return _phaseGenerator(RandomDevice); });
	double x;
	meter.measure([&x,&periods,&phases](int i)
	{
		x = syn::lut_bl_saw_table().getresampled(phases[i],periods[i]);
	});
})

NONIUS_BENCHMARK("naive saw", [](nonius::chronometer& meter) {
	std::vector<double> phases(meter.runs());
	std::uniform_real_distribution<> _phaseGenerator(0.0, 1.0);
	std::transform(phases.begin(), phases.end(), phases.begin(), [&_phaseGenerator](double& x) {return _phaseGenerator(RandomDevice); });
	double x;
	meter.measure([&x, &phases](int i)
	{
		x = syn::naive_saw(phases[i]);
	});
})
#endif

#if lut_pitch_benches
NONIUS_BENCHMARK("lut pitch2freq", [](nonius::chronometer& meter) {
	std::vector<double> pitches(meter.runs());
	std::uniform_int_distribution<> _pitchGenerator(-128, 128);
	std::transform(pitches.begin(), pitches.end(), pitches.begin(), [&_pitchGenerator](double& x) {return _pitchGenerator(RandomDevice); });
	double x;
	meter.measure([&x, &pitches](int i)
	{
		x = syn::pitchToFreq(pitches[i]);
	});
})

NONIUS_BENCHMARK("naive pitch2freq", [](nonius::chronometer& meter) {
	std::vector<double> pitches(meter.runs());
	std::uniform_int_distribution<> _pitchGenerator(-128, 128);
	std::transform(pitches.begin(), pitches.end(), pitches.begin(), [&_pitchGenerator](double& x) {return _pitchGenerator(RandomDevice); });
	double x;
	meter.measure([&x, &pitches](int i)
	{
		x = syn::naive_pitchToFreq(pitches[i]);
	});
})
#endif

#if container_benches
#define container_size 1024
NONIUS_BENCHMARK("NamedContainer []", [](nonius::chronometer& meter)
{
	std::uniform_int_distribution<> _accessGenerator(0, container_size-1);
	std::vector<int> loads(meter.runs());
	std::vector<int> stores(meter.runs());
	std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
	std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

	syn::NamedContainer<syn::UnitPort, container_size> myContainer;
	for (int i = 0; i < container_size; i++) myContainer.add(std::to_string(i), syn::UnitPort(i));

	meter.measure([&myContainer,&stores,&loads](int i)
	{
		myContainer[stores[i]] = myContainer[loads[i]];
	});
})

NONIUS_BENCHMARK("Array []", [](nonius::chronometer& meter)
{
	std::uniform_int_distribution<> _accessGenerator(0, container_size-1);
	std::vector<int> loads(meter.runs());
	std::vector<int> stores(meter.runs());
	std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
	std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

	std::array<syn::UnitPort, container_size> myContainer;
	for (int i = 0; i < container_size; i++) myContainer[i]=syn::UnitPort(i);

	meter.measure([&myContainer, &stores, &loads](int i)
	{
		myContainer[stores[i]] = myContainer[loads[i]];
	});
})

NONIUS_BENCHMARK("Vector []", [](nonius::chronometer& meter)
{
	std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
	std::vector<int> loads(meter.runs());
	std::vector<int> stores(meter.runs());
	std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
	std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

	std::vector<syn::UnitPort> myContainer(container_size);
	for (int i = 0; i < container_size; i++) myContainer[i] = syn::UnitPort(i);

	meter.measure([&myContainer, &stores, &loads](int i)
	{
		myContainer[stores[i]] = myContainer[loads[i]];
	});
})

NONIUS_BENCHMARK("Map [int]", [](nonius::chronometer& meter)
{
	std::uniform_int_distribution<> _accessGenerator(0, container_size - 1);
	std::vector<int> loads(meter.runs());
	std::vector<int> stores(meter.runs());
	std::transform(loads.begin(), loads.end(), loads.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });
	std::transform(stores.begin(), stores.end(), stores.begin(), [&_accessGenerator](int& x) {return _accessGenerator(RandomDevice); });

	std::map<int, syn::UnitPort> myContainer;
	for (int i = 0; i < container_size; i++) myContainer[i] = syn::UnitPort(i);

	meter.measure([&myContainer, &stores, &loads](int i)
	{
		myContainer[stores[i]] = myContainer[loads[i]];
	});
})
#endif