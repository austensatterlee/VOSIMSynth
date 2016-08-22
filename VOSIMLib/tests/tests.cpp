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
	vector<double> phases(N);
	vector<double> base_tanh_results(N);
	vector<double> fast_tanh_poly_results(N);
	uniform_real_distribution<> _phaseGenerator(-4.0, 4.0);
	std::transform(&phases[0], &phases[N-1], &phases[0], [&_phaseGenerator](double& x)->double {return _phaseGenerator(RandomDevice); });
	double avg_error = 0;
	for(int i=0;i<N;i++) {		
		base_tanh_results[i] = std::tanh(phases[i]);
		fast_tanh_poly_results[i] = syn::fast_tanh_poly<double>(phases[i]);
		avg_error += abs(base_tanh_results[i] - fast_tanh_poly_results[i]);
	}
	avg_error *= 1./N;
	std::cout << "Error: " << avg_error << std::endl;
	return avg_error;
}

int main() 
{
	test_fasttanh_poly();
	test_serialization();
	return 0;
}
