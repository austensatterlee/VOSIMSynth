/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

#include "StateVariableFilter.h"
#include "tables.h"
#include "DSPMath.h"

syn::StateVariableFilter::StateVariableFilter(const string& a_name):
	Unit(a_name),
	m_pFc(addParameter_(UnitParameter("fc", 0.0, 1.0, 0.5))),
	m_pRes(addParameter_(UnitParameter("res", 0.0, 1.0, 0.0))),
	m_prevBPOut(0.0), m_prevLPOut(0.0),
	m_F(0.0), m_Q(0.0) {
	addInput_("in");
	m_iFcAdd = addInput_("fc");
	m_iFcMul = addInput_("fc[x]", 1.0, Signal::EMul);
	m_iResAdd = addInput_("res");
	m_iResMul = addInput_("res[x]", 1.0, Signal::EMul);
	m_oLP = addOutput_("LP");
	m_oHP = addOutput_("HP");
	m_oBP = addOutput_("BP");
	m_oN = addOutput_("N");
}

void syn::StateVariableFilter::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
	double input_fc = a_inputs.getValue(m_iFcMul) * getParameter(m_pFc).getDouble() + a_inputs.getValue(m_iFcAdd);
	input_fc = CLAMP(input_fc, 0, 1) * 128;
	double fc = pitchToFreq(input_fc);
	m_F = 2 * lut_sin.getlinear(0.5 * fc / (getFs() * c_oversamplingFactor));

	double input_res = a_inputs.getValue(m_iResMul) * getParameter(m_pRes).getDouble() + a_inputs.getValue(m_iResAdd);
	input_res = CLAMP(input_res, 0, 1);
	double res = LERP(c_minRes, c_maxRes, input_res);
	m_Q = 1.0 / res;

	double input = a_inputs.getValue(0);
	double LPOut = 0, HPOut = 0, BPOut = 0;
	int i = c_oversamplingFactor;
	while (i--) {
		LPOut = m_prevLPOut + m_F * m_prevBPOut;
		HPOut = input - LPOut - m_Q * m_prevBPOut;
		BPOut = m_F * HPOut + m_prevBPOut;

		m_prevBPOut = BPOut;
		m_prevLPOut = LPOut;
	}
	double NOut = HPOut + LPOut;

	a_outputs.setChannel(m_oLP, LPOut);
	a_outputs.setChannel(m_oHP, HPOut);
	a_outputs.setChannel(m_oBP, BPOut);
	a_outputs.setChannel(m_oN, NOut);
}
