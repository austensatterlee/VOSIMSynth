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

#ifndef lut_sin_benches
#define lut_sin_benches 1
#endif

#ifndef lut_saw_benches
#define lut_saw_benches 1
#endif

#ifndef lut_pitch_benches
#define lut_pitch_benches 1
#endif

#ifndef container_benches
#define container_benches 1
#endif

std::random_device RandomDevice;

#if lut_sin_benches
NONIUS_BENCHMARK("lut sin (raw)", [](nonius::chronometer& meter) {
	double x;
	meter.measure([&x](int i) { x = syn::lut_sin_table().getraw(syn::WRAP<int>(i * (1024.0 / 48e3), 1024)); });
})

NONIUS_BENCHMARK("lut sin (lin)", [](nonius::chronometer& meter) {
	double x;
	meter.measure([&x](int i) { x = syn::lut_sin_table().getlinear(syn::WRAP<double>(i / 48e3, 1)); });
})

NONIUS_BENCHMARK("std::sin", [](nonius::chronometer& meter) {
	double x;
	meter.measure([&x](int i) { x = std::sin(2*DSP_PI*syn::WRAP<double>(i / 48e3, 1)); });
})
#endif

#if lut_saw_benches
NONIUS_BENCHMARK("lut saw", [](nonius::chronometer& meter) {
	std::vector<int> periods(meter.runs());
	std::vector<int> phases(meter.runs());
	std::uniform_int_distribution<> _periodGenerator(2, syn::lut_bl_saw_table().size() - 1);
	std::uniform_real_distribution<> _phaseGenerator(0.0,1.0);
	std::transform(periods.begin(), periods.end(), periods.begin(), [&_periodGenerator](int& x) {return _periodGenerator(RandomDevice); });
	std::transform(phases.begin(), phases.end(), phases.begin(), [&_phaseGenerator](int& x) {return _phaseGenerator(RandomDevice); });
	double x;
	meter.measure([&x,&periods,&phases](int i)
	{
		x = syn::lut_bl_saw_table().getresampled(phases[i],periods[i]);
	});
})

NONIUS_BENCHMARK("naive saw", [](nonius::chronometer& meter) {
	std::vector<int> phases(meter.runs());
	std::uniform_real_distribution<> _phaseGenerator(0.0, 1.0);
	std::transform(phases.begin(), phases.end(), phases.begin(), [&_phaseGenerator](int& x) {return _phaseGenerator(RandomDevice); });
	double x;
	meter.measure([&x, &phases](int i)
	{
		x = syn::naive_saw(phases[i]);
	});
})
#endif

#if lut_pitch_benches
NONIUS_BENCHMARK("lut pitch2freq", [](nonius::chronometer& meter) {
	std::vector<int> pitches(meter.runs());
	std::uniform_int_distribution<> _pitchGenerator(-128, 128);
	std::transform(pitches.begin(), pitches.end(), pitches.begin(), [&_pitchGenerator](int& x) {return _pitchGenerator(RandomDevice); });
	double x;
	meter.measure([&x, &pitches](int i)
	{
		x = syn::pitchToFreq(pitches[i]);
	});
})

NONIUS_BENCHMARK("naive pitch2freq", [](nonius::chronometer& meter) {
	std::vector<int> pitches(meter.runs());
	std::uniform_int_distribution<> _pitchGenerator(-128, 128);
	std::transform(pitches.begin(), pitches.end(), pitches.begin(), [&_pitchGenerator](int& x) {return _pitchGenerator(RandomDevice); });
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