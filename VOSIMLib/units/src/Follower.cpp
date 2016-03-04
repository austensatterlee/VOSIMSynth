#include "Follower.h"

syn::FollowerUnit::FollowerUnit(const string& a_name):
	Unit(a_name),
	m_w(0.0),
	m_output(0.0),
	m_pAlpha(addParameter_(UnitParameter("alpha",0.0,1.0,0.9))),
	m_pBeta(addParameter_(UnitParameter("beta", 0.0,1.0,0.01)))
{
	addInput_("in");
	addOutput_("out");
}

syn::FollowerUnit::FollowerUnit(const FollowerUnit& a_rhs):
	FollowerUnit(a_rhs.getName()) {}

void syn::FollowerUnit::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
	double input = a_inputs.getValue(0);
	double alpha = getParameter(m_pAlpha).getDouble();
	double beta = getParameter(m_pBeta).getDouble();
	// dc removal + rectification
	double old_w = m_w;
	m_w = input + alpha*old_w;
	double hpOutput = abs(m_w - old_w);
	// low pass
	m_output = beta*hpOutput + (1 - beta)*m_output;
	a_outputs.setChannel(0, sqrt(m_output));
}
