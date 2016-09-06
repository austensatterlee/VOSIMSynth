#include "WaveShapers.h"
#include "common.h"
CEREAL_REGISTER_ARCHIVE(syn::TanhUnit);

syn::TanhUnit::TanhUnit(const string& a_name): Unit(a_name) {
	addParameter_(pSat, UnitParameter("sat", 1.0, 10.0, 1.0));
	addInput_("in");
	addOutput_("out");
}

syn::TanhUnit::TanhUnit(const TanhUnit& a_rhs): TanhUnit(a_rhs.getName()) {}

void syn::TanhUnit::process_() {
	double input = getInputValue(0);
	double sat = getParameter(pSat).getDouble();
	setOutputChannel_(0, fast_tanh_rat(input * sat) / fast_tanh_rat(sat));
}
