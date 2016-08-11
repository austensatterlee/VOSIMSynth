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

syn::StateVariableFilter::StateVariableFilter(const string& a_name) :
	Unit(a_name),
	m_pFc(addParameter_(UnitParameter("fc", 0.01, 20000.0, 10000.0, UnitParameter::Freq))),
	m_pRes(addParameter_(UnitParameter("res", 0.0, 1.0, 0.0))),
	m_prevBPOut(0.0), m_prevLPOut(0.0),
	m_F(0.0), m_damp(0.0) {
	addInput_("in");
	m_iFcAdd = addInput_("fc");
	m_iFcMul = addInput_("fc[x]", 1.0);
	m_iResAdd = addInput_("res");
	m_iResMul = addInput_("res[x]", 1.0);
	m_oLP = addOutput_("LP");
	m_oHP = addOutput_("HP");
	m_oBP = addOutput_("BP");
	m_oN = addOutput_("N");
}

void syn::StateVariableFilter::process_() {
	double fc = getInputValue(m_iFcMul) * (getParameter(m_pFc).getDouble() + getInputValue(m_iFcAdd));
	fc = CLAMP(fc, getParameter(m_pFc).getMin(), getParameter(m_pFc).getMax());
	m_F = 2 * lut_sin_table().getlinear(0.5 * fc / (getFs() * c_oversamplingFactor));

	double input_res = getInputValue(m_iResMul) * getParameter(m_pRes).getDouble() + getInputValue(m_iResAdd);
	input_res = CLAMP<double>(input_res, 0, 1);
	double res = LERP(c_minRes, c_maxRes, input_res);
	m_damp = 1.0 / res;

	double input = getInputValue(0);
	double LPOut = 0, HPOut = 0, BPOut = 0;
	int i = c_oversamplingFactor;
	while (i--) {
		LPOut = m_prevLPOut + m_F * m_prevBPOut;
		HPOut = input - LPOut - m_damp * m_prevBPOut;
		BPOut = m_F * HPOut + m_prevBPOut;

		m_prevBPOut = BPOut;
		m_prevLPOut = LPOut;
	}
	double NOut = HPOut + LPOut;

	setOutputChannel_(m_oLP, LPOut);
	setOutputChannel_(m_oHP, HPOut);
	setOutputChannel_(m_oBP, BPOut);
	setOutputChannel_(m_oN, NOut);
}

syn::TrapStateVariableFilter::TrapStateVariableFilter(const string& a_name) : StateVariableFilter(a_name), m_prevInput(0.0) {}

void syn::TrapStateVariableFilter::process_() {
	double fc = getInputValue(m_iFcMul) * (getParameter(m_pFc).getDouble() + getInputValue(m_iFcAdd));
	fc = CLAMP(fc, getParameter(m_pFc).getMin(), getParameter(m_pFc).getMax());
	m_F = tan(DSP_PI * fc / (getFs() * c_oversamplingFactor));

	double input_res = getInputValue(m_iResMul) * getParameter(m_pRes).getDouble() + getInputValue(m_iResAdd);
	input_res = CLAMP<double>(input_res, 0, 1);
	double res = LERP(c_minRes, c_maxRes, input_res);
	m_damp = 1.0 / res;

	double input = getInputValue(0);
	double LPOut = 0, HPOut = 0, BPOut = 0, NOut = 0;
	int i = c_oversamplingFactor;
	while (i--) {
		BPOut = (m_prevBPOut + m_F*(input + m_prevInput - (m_F + m_damp)*(m_prevBPOut)-2 * m_prevLPOut)) / (1 + m_F*(m_F + m_damp));
		LPOut = (m_prevLPOut + m_F*(2 * m_prevBPOut + m_F*(input + m_prevInput - m_prevLPOut) + m_damp * m_prevLPOut)) / (1 + m_F*(m_F + m_damp));
		m_prevInput = input;
		m_prevBPOut = BPOut;
		m_prevLPOut = LPOut;
	}
	HPOut = input - m_damp * BPOut - LPOut;
	NOut = HPOut + LPOut;
	setOutputChannel_(m_oLP, LPOut);
	setOutputChannel_(m_oHP, HPOut);
	setOutputChannel_(m_oBP, BPOut);
	setOutputChannel_(m_oN, NOut);
}


syn::OnePoleLP::OnePoleLP(const string& a_name) :
	Unit(a_name),
	m_state(0.0)
{
	addParameter_(pFc, UnitParameter("fc", 0.01, 20000.0, 1.0, UnitParameter::Freq));
	addInput_(iAudioIn, "in");
	addInput_(iFcAdd, "fc");
	addInput_(iFcMul, "fc[x]", 1.0);
	addOutput_("out");
}

double syn::OnePoleLP::getState() const {
	return m_state;
}

void syn::OnePoleLP::process_() {
	// Calculate gain for specified cutoff
	double fc = getParameter(pFc).getDouble() * getInputValue(iFcMul) + getInputValue(iFcAdd); // freq cutoff
	fc = CLAMP(fc, getParameter(pFc).getMin(), getParameter(pFc).getMax());
	fc = fc / getFs();
	double wc = 2 * tan(DSP_PI * fc / 2.0);
	double gain = wc / (2 + wc);

	// Calculate output
	double input = getInputValue(0);	
	double trap_in = gain * (input - m_state);
	double output = trap_in + m_state;
	m_state = trap_in + output;
	setOutputChannel_(0, output);
}

syn::LadderFilter::LadderFilter(const string& a_name) :
	Unit(a_name),
	m_u(0.0)
{
	addParameter_(pFc, UnitParameter("fc", 0.01, 20000.0, 1.0, UnitParameter::Freq));
	addParameter_(pFb, UnitParameter("fb", 0.0, 1.0, 0.0));
	addInput_(iAudioIn, "in");
	addInput_(iFcAdd, "fc");
	addInput_(iFcMul, "fc[x]", 1.0);
	addOutput_("out");

	m_ladder[0].connectInput(OnePoleLP::iAudioIn, &m_u);
	m_ladder[1].connectInput(OnePoleLP::iAudioIn, &m_ladder[0].getOutputValue(0));
	m_ladder[2].connectInput(OnePoleLP::iAudioIn, &m_ladder[1].getOutputValue(0));
	m_ladder[3].connectInput(OnePoleLP::iAudioIn, &m_ladder[2].getOutputValue(0));
}

void syn::LadderFilter::process_() {
	// Calculate gain for specified cutoff
	double fc = getParameter(pFc).getDouble() * getInputValue(iFcMul) + getInputValue(iFcAdd); // freq cutoff
	fc = CLAMP(fc, getParameter(pFc).getMin(), getParameter(pFc).getMax());
	fc = fc / getFs();
	double wc = 2 * tan(DSP_PI * fc / 2.0);

	// Calculate output
	double g1 = wc / (1 + wc);
	double g2 = g1*g1;
	double g3 = g1*g2;
	double g4 = g1*g3;
	double S = g3 * m_ladder[0].getState() + g2 * m_ladder[1].getState() + g1 * m_ladder[2].getState() + m_ladder[3].getState();
	double k = getParameter(pFb).getDouble();
	m_u = (getInputValue(0) - k * S) / (1 + k*g4);
	m_ladder[0].tick();
	m_ladder[1].tick();
	m_ladder[2].tick();
	m_ladder[3].tick();
	setOutputChannel_(0, m_ladder[3].getOutputValue(0));
}

void syn::LadderFilter::onParamChange_(int a_paramId) {
	if(a_paramId==pFc) {
		m_ladder[0].setParameterValue(OnePoleLP::pFc, getParameter(pFc).getDouble());
		m_ladder[1].setParameterValue(OnePoleLP::pFc, getParameter(pFc).getDouble());
		m_ladder[2].setParameterValue(OnePoleLP::pFc, getParameter(pFc).getDouble());
		m_ladder[3].setParameterValue(OnePoleLP::pFc, getParameter(pFc).getDouble());
	}
}

void syn::LadderFilter::onInputConnection_(int a_inputPort) {
	if (a_inputPort == iFcAdd) {
		m_ladder[0].connectInput(OnePoleLP::iFcAdd, getInputSource(iFcAdd));
		m_ladder[1].connectInput(OnePoleLP::iFcAdd, getInputSource(iFcAdd));
		m_ladder[2].connectInput(OnePoleLP::iFcAdd, getInputSource(iFcAdd));
		m_ladder[3].connectInput(OnePoleLP::iFcAdd, getInputSource(iFcAdd));
	} else if(a_inputPort == iFcMul) {
		m_ladder[0].connectInput(OnePoleLP::iFcMul, getInputSource(iFcMul));
		m_ladder[1].connectInput(OnePoleLP::iFcMul, getInputSource(iFcMul));
		m_ladder[2].connectInput(OnePoleLP::iFcMul, getInputSource(iFcMul));
		m_ladder[3].connectInput(OnePoleLP::iFcMul, getInputSource(iFcMul));
	}
}

void syn::LadderFilter::onInputDisconnection_(int a_inputPort) {
	if (a_inputPort == iFcAdd) {
		m_ladder[0].connectInput(OnePoleLP::iFcAdd, getInputSource(iFcAdd));
		m_ladder[1].connectInput(OnePoleLP::iFcAdd, getInputSource(iFcAdd));
		m_ladder[2].connectInput(OnePoleLP::iFcAdd, getInputSource(iFcAdd));
		m_ladder[3].connectInput(OnePoleLP::iFcAdd, getInputSource(iFcAdd));
	} else if (a_inputPort == iFcMul) {
		m_ladder[0].connectInput(OnePoleLP::iFcMul, getInputSource(iFcMul));
		m_ladder[1].connectInput(OnePoleLP::iFcMul, getInputSource(iFcMul));
		m_ladder[2].connectInput(OnePoleLP::iFcMul, getInputSource(iFcMul));
		m_ladder[3].connectInput(OnePoleLP::iFcMul, getInputSource(iFcMul));
	}
}

void syn::LadderFilter::onFsChange_() {
	m_ladder[0].setFs(getFs());
	m_ladder[1].setFs(getFs());
	m_ladder[2].setFs(getFs());
	m_ladder[3].setFs(getFs());
}
