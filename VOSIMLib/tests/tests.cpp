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

void test_serialization(){
	syn::Circuit* circ = new syn::Circuit("main");
	syn::Unit* svfUnit = new syn::StateVariableFilter("svfUnit");
	syn::Unit* oscUnit = new syn::BasicOscillator("oscUnit");
	circ->addUnit(svfUnit);
	circ->addUnit(oscUnit);
	circ->connectInternal("oscUnit",0,"svfUnit",0);
	std::stringstream ss;
	{
		cereal::XMLOutputArchive oarchive(ss);
		shared_ptr<syn::Unit> tmpUnit(circ->clone());
		oarchive(tmpUnit);
	}
	std::cout << ss.str() << std::endl;
	std::cout << "----------" << std::endl;

	syn::Unit* readUnit;
	{
		cereal::XMLInputArchive iarchive(ss);
		shared_ptr<syn::Unit> tmpUnit;
		iarchive(tmpUnit);
		readUnit = tmpUnit->clone();
	}

	std::stringstream ss2;
	{
		cereal::XMLOutputArchive oarchive(ss2);
		shared_ptr<syn::Unit> tmpUnit(circ->clone());
		oarchive(tmpUnit);
	}
	std::cout << ss2.str() << std::endl;
}

double test_fasttanh_poly() {
	const int N = 48000;
	vector<double> base_tanh_results(N);
	vector<double> fast_tanh_poly_results(N);
	double avg_error = 0;
	double phase;
	for(int i=0;i<N;i++) {		
		phase = i*8.0 / N - 4;
		base_tanh_results[i] = std::tanh(phase);
		fast_tanh_poly_results[i] = syn::fast_tanh_poly<double>(phase);
		avg_error += abs(base_tanh_results[i] - fast_tanh_poly_results[i]);
	}
	avg_error *= 1./N;
	std::cout << "Error: " << avg_error << std::endl;
	return avg_error;
}

double test_denormals() {
	const double epsilon = std::numeric_limits<double>::min();
	const int N = 2.88e6;
	const double fs = 48e3;
	syn::LadderFilter ladder("ladder");
	double input = 1.0;
	ladder.setFs(fs);
	ladder.setParameterValue(syn::LadderFilter::pFc, 1000.0);
	ladder.connectInput(0, &input);

	double x;
	int ndenormals = 0;
	for(int i=0;i<N;i++)
	{
		input = 1.0;
		ladder.tick();
		x = ladder.getOutputValue(0);
		if (std::abs(x) < epsilon) ndenormals++;
		input = 0.0;
	}
	std::cout << "# Denormals: " << ndenormals << "/" << N << std::endl;
	return ndenormals * 1.0 / N;
}

int main() 
{
	test_denormals();
	test_fasttanh_poly();
	//test_serialization();
	return 0;
}
