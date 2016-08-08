#define NONIUS_RUNNER
#include <nonius.h++>
#include "tables.h"
#include <DSPMath.h>

NONIUS_BENCHMARK("lut sin (raw)", [](nonius::chronometer& meter) {
	double x;
	meter.measure([&x](int i) { x = syn::lut_sin.getraw(syn::WRAP<int>(i * (1024.0 / 48e3), 1024)); });
})

NONIUS_BENCHMARK("lut sin (lin)", [](nonius::chronometer& meter) {
	double x;
	meter.measure([&x](int i) { x = syn::lut_sin.getlinear(syn::WRAP<double>(i / 48e3, 1)); });
})