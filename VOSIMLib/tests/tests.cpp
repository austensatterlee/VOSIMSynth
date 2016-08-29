#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include "Unit.h"
#include "StateVariableFilter.h"
#include "Oscillator.h"
#include "Circuit.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <random>
#include <cmath>
#include <DSPMath.h>

std::random_device RandomDevice;

template <typename T>
T detect_denormals(const std::vector<T>& a_data) {
	const double epsilon = std::numeric_limits<double>::min();
	int ndenormals = 0;
	for (const T& x : a_data) {
		if (std::abs(x) < epsilon)
			ndenormals++;
	}
	return ndenormals;
}

void serialization_example() {
	syn::Circuit* circ = new syn::Circuit("main");
	syn::Unit* svfUnit = new syn::StateVariableFilter("svfUnit");
	syn::Unit* oscUnit = new syn::BasicOscillator("oscUnit");
	circ->addUnit(svfUnit);
	circ->addUnit(oscUnit);
	circ->connectInternal("oscUnit", 0, "svfUnit", 0);
	std::stringstream ss; {
		cereal::XMLOutputArchive oarchive(ss);
		shared_ptr<syn::Unit> tmpUnit(circ->clone());
		oarchive(tmpUnit);
	}
	std::cout << ss.str() << std::endl;
	std::cout << "----------" << std::endl;

	syn::Unit* readUnit; {
		cereal::XMLInputArchive iarchive(ss);
		shared_ptr<syn::Unit> tmpUnit;
		iarchive(tmpUnit);
		readUnit = tmpUnit->clone();
	}

	std::stringstream ss2; {
		cereal::XMLOutputArchive oarchive(ss2);
		shared_ptr<syn::Unit> tmpUnit(circ->clone());
		oarchive(tmpUnit);
	}
	std::cout << ss2.str() << std::endl;
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

TEST_CASE("Measure error for rational approximations of tanh.",	"[fast-tanh]") {
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

TEST_CASE("Detect subnormals in ladder filter processing.",	"[ladder-denormal]") {
	const int N = 2.88e6;
	const double fs = 48e3;
	syn::LadderFilter ladder("ladder");
	double input = 1.0;
	ladder.setFs(fs);
	ladder.setParameterValue(syn::LadderFilter::pFc, 1.0);
	ladder.setParameterValue(syn::LadderFilter::pFb, 1.0);
	ladder.setParameterValue(syn::LadderFilter::pDrv, 1.0);
	ladder.connectInput(0, &input);

	vector<double> ladder_out(N);
	for (int i = 0; i < N; i++) {
		ladder.tick();
		ladder_out[i] = ladder.getOutputValue(0);
		input = 0.0;
	}

	int ndenormals = detect_denormals(ladder_out);
	INFO( "Denormal count: " << ndenormals << "/" << N);
	REQUIRE(ndenormals == 0);
}