#include "Follower.h"

syn::FollowerUnit::FollowerUnit(const string& a_name):
	Unit(a_name) {
	addInput_("in");
	addOutput_("out");
}

syn::FollowerUnit::FollowerUnit(const FollowerUnit& a_rhs):
	FollowerUnit(a_rhs.getName()) {

}

void syn::FollowerUnit::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
	double input = a_inputs.getValue(0);
	a_outputs.setChannel(0, abs(input));
}
