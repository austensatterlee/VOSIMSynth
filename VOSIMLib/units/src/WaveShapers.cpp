#include "WaveShapers.h"
#include "common.h"


syn::TanhUnit::TanhUnit(const string& a_name): Unit(a_name) {
    addParameter_(pSat, UnitParameter("sat", 1.0, 10.0, 1.0));
    addInput_("in");
    addOutput_("out");
}

syn::TanhUnit::TanhUnit(const TanhUnit& a_rhs): TanhUnit(a_rhs.name()) {}

void syn::TanhUnit::process_() {
    double input = readInput(0);
    double sat = param(pSat).getDouble();
    setOutputChannel_(0, fast_tanh_rat(input * sat) / fast_tanh_rat(sat));
}
