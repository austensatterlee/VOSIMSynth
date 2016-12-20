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
#include "DSPMath.h"
#include "tables.h"

#include "common.h"
CEREAL_REGISTER_TYPE(syn::StateVariableFilter);
CEREAL_REGISTER_TYPE(syn::TrapStateVariableFilter);
CEREAL_REGISTER_TYPE(syn::OnePoleLP);
CEREAL_REGISTER_TYPE(syn::LadderFilter);
CEREAL_REGISTER_TYPE(syn::LadderFilterTwo);

syn::StateVariableFilter::StateVariableFilter(const string &a_name) :
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

void syn::StateVariableFilter::reset() {
	m_prevBPOut = 0.0;
	m_prevLPOut = 0.0;
}

void syn::StateVariableFilter::process_() {
  double fc = readInput(m_iFcMul) * (param(m_pFc).getDouble() + readInput(m_iFcAdd));
  fc = CLAMP(fc, param(m_pFc).getMin(), param(m_pFc).getMax());
  m_F = 2 * lut_sin_table().getlinear_periodic(0.5 * fc / (fs() * c_oversamplingFactor));

  double input_res = readInput(m_iResMul) * param(m_pRes).getDouble() + readInput(m_iResAdd);
  input_res = CLAMP<double>(input_res, 0, 1);
  double res = LERP(c_minRes, c_maxRes, input_res);
  m_damp = 1.0 / res;

  double input = readInput(0);
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

syn::TrapStateVariableFilter::TrapStateVariableFilter(const string &a_name) : StateVariableFilter(a_name),
  m_prevInput(0.0) 
{}

void syn::TrapStateVariableFilter::reset() {
	StateVariableFilter::reset();
	m_prevInput = 0.0;
}

void syn::TrapStateVariableFilter::process_() {
  double fc = readInput(m_iFcMul) * (param(m_pFc).getDouble() + readInput(m_iFcAdd));
  fc = CLAMP(fc, param(m_pFc).getMin(), param(m_pFc).getMax());
  m_F = tan(DSP_PI * fc / (fs() * c_oversamplingFactor));

  double input_res = readInput(m_iResMul) * param(m_pRes).getDouble() + readInput(m_iResAdd);
  input_res = CLAMP<double>(input_res, 0, 1);
  double res = LERP(c_minRes, c_maxRes, input_res);
  m_damp = 1.0 / res;

  double input = readInput(0);
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

void syn::_OnePoleLP::setFc(double a_fc, double a_fs) {
  a_fc = DSP_PI * a_fc / a_fs;
  double g = tan(a_fc);

  m_G = g / (1 + g);
}

double syn::_OnePoleLP::process(double a_input) {
  double trap_in = m_G * (a_input - m_state);
  double output = trap_in + m_state;
  m_state = trap_in + output;
  return output;
}

void syn::_OnePoleLP::reset() {
  m_state = 0.0;
}

syn::OnePoleLP::OnePoleLP(const string &a_name) :
  Unit(a_name)
{
  addParameter_(pFc, UnitParameter("fc", 0.01, 20000.0, 1.0, UnitParameter::Freq));
  addInput_(iAudioIn, "in");
  addInput_(iFcAdd, "fc");
  addInput_(iFcMul, "fc[x]", 1.0);
  addOutput_(oLP, "LP");
  addOutput_(oHP, "HP");
}

double syn::OnePoleLP::getState() const {
  return implem.m_state;
}

void syn::OnePoleLP::reset() {
	implem.reset();
}

void syn::OnePoleLP::process_() {
  // Calculate gain for specified cutoff
  double fc = (param(pFc).getDouble() + readInput(iFcAdd)) * readInput(iFcMul); // freq cutoff
  fc = CLAMP(fc, param(pFc).getMin(), param(pFc).getMax());
  implem.setFc(fc, fs());

  double input = readInput(0);
  double output = implem.process(input);
  setOutputChannel_(oLP, output);
  setOutputChannel_(oHP, input - output);
}

syn::LadderFilterBase::LadderFilterBase(const string &a_name) : Unit(a_name)
{
	addParameter_(pFc, UnitParameter("fc", 0.01, 20000.0, 10000.0, UnitParameter::Freq));
	addParameter_(pFb, UnitParameter("res", 0.0, 1.0, 0.0));
	addParameter_(pDrv, UnitParameter("drv", 0.0, 1.0, 0.0));
	addInput_(iAudioIn, "in");
	addInput_(iFcAdd, "fc");
	addInput_(iFcMul, "fc[x]", 1.0);
	addOutput_("out");
}


syn::LadderFilter::LadderFilter(const string &a_name) :
	LadderFilterBase(a_name)
{}

void syn::LadderFilter::reset() {
	m_V.fill(0.0);
	m_dV.fill(0.0);
	m_tV.fill(0.0);
}

void syn::LadderFilter::process_() {
  double input = readInput(iAudioIn);
  // Calculate gain for specified cutoff
  double fs = LadderFilter::fs() * c_oversamplingFactor;

  double fc = (param(pFc).getDouble() + readInput(iFcAdd)) * readInput(iFcMul); // freq cutoff
  fc = CLAMP(fc, param(pFc).getMin(), param(pFc).getMax());

  double wd = DSP_PI * fc / fs;

  // Prepare parameter values and insert them into each stage.
  double g = 4 * DSP_PI * VT * fc * (1.0 - wd) / (1.0 + wd);
  double dV0, dV1, dV2, dV3;
  double drive = 1+3*param(pDrv).getDouble();
  double res = 3.9*param(pFb).getDouble();

  int i = c_oversamplingFactor;
  while (i--) {
    dV0 = -g * (fast_tanh_rat((drive * input + res * m_V[3]) / (2.0 * VT)) + m_tV[0]);
    m_V[0] += (dV0 + m_dV[0]) / (2.0 * fs);
    m_dV[0] = dV0;
    m_tV[0] = fast_tanh_rat(m_V[0] / (2.0 * VT));

    dV1 = g * (m_tV[0] - m_tV[1]);
    m_V[1] += (dV1 + m_dV[1]) / (2.0 * fs);
    m_dV[1] = dV1;
    m_tV[1] = fast_tanh_rat(m_V[1] / (2.0 * VT));

    dV2 = g * (m_tV[1] - m_tV[2]);
    m_V[2] += (dV2 + m_dV[2]) / (2.0 * fs);
    m_dV[2] = dV2;
    m_tV[2] = fast_tanh_rat(m_V[2] / (2.0 * VT));

    dV3 = g * (m_tV[2] - m_tV[3]);
    m_V[3] += (dV3 + m_dV[3]) / (2.0 * fs);
    m_dV[3] = dV3;
    m_tV[3] = fast_tanh_rat(m_V[3] / (2.0 * VT));
  }

  setOutputChannel_(0, m_V[3]);
}

syn::LadderFilterTwo::LadderFilterTwo(const string &a_name) : LadderFilterBase(a_name),
	m_ffGains(Eigen::Matrix<double, 5, 1>::Unit(4))
{}

void syn::LadderFilterTwo::reset() {
	m_LP[0].reset();
	m_LP[1].reset();
	m_LP[2].reset();
	m_LP[3].reset();
}

void syn::LadderFilterTwo::process_() {
	double input = readInput(iAudioIn);
	// Calculate gain for specified cutoff
	double fs = LadderFilterTwo::fs() * c_oversamplingFactor;

	double fc, drive, res;
	fc = (param(pFc).getDouble() + readInput(iFcAdd)) * readInput(iFcMul); // freq cutoff
	fc = CLAMP(fc, param(pFc).getMin(), param(pFc).getMax());
	drive = 1.0 + 3.0 * param(pDrv).getDouble();
	res = 3.9*param(pFb).getDouble();

	m_LP[0].setFc(fc, fs);
	m_LP[1].m_G = m_LP[0].m_G;
	m_LP[2].m_G = m_LP[0].m_G;
	m_LP[3].m_G = m_LP[0].m_G;

	double g = m_LP[0].m_G / (1 - m_LP[0].m_G);
	double lp3_fb_gain = 1.0 / (1.0 + g);
	double lp2_fb_gain = m_LP[0].m_G * lp3_fb_gain;
	double lp1_fb_gain = m_LP[0].m_G * lp2_fb_gain;
	double lp0_fb_gain = m_LP[0].m_G * lp1_fb_gain;

	double G4 = m_LP[0].m_G*m_LP[0].m_G*m_LP[0].m_G*m_LP[0].m_G;
	double out_fb_gain = 1.0 / (1.0 + res*G4);

	int i = c_oversamplingFactor;
	Vector5d out_states;
	input *= drive;
	while (i--) {
		double out_fb = lp0_fb_gain*m_LP[0].m_state + lp1_fb_gain*m_LP[1].m_state + lp2_fb_gain*m_LP[2].m_state + lp3_fb_gain*m_LP[3].m_state;

		out_states[0] = fast_tanh_rat((input - res*(out_fb - input)) * out_fb_gain);
		out_states[1] = m_LP[0].process(out_states[0]);
		out_states[2] = m_LP[1].process(out_states[1]);
		out_states[3] = m_LP[2].process(out_states[2]);
		out_states[4] = m_LP[3].process(out_states[3]);
	}
	double out = m_ffGains.dot(out_states);
	setOutputChannel_(0, out);
}	
