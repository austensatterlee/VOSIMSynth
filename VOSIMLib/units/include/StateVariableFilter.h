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

/**
* \file StateVariableFilter.h
* \brief
* \details
* \author Austen Satterlee
* \date March 11, 2016
*/
#ifndef __STATEVARIABLEFILTER__
#define __STATEVARIABLEFILTER__
#include "Unit.h"
#include <tables.h>
#include <DSPMath.h>

using namespace std;

namespace syn
{
	class StateVariableFilter : public Unit
	{
	public:
		StateVariableFilter(const string& a_name) :
			Unit(a_name),
			m_pFc(addParameter_(UnitParameter("fc", 0.0, 1.0, 0.5))),
			m_pRes(addParameter_(UnitParameter("res", 0.0, 1.0, 0.0))),
			m_prevBPOut(0.0),m_prevLPOut(0.0) 
		{
			addInput_("in");
			m_iFcAdd = addInput_("fc");
			m_iFcMul = addInput_("fc[x]", 1.0, Signal::EMul);
			m_iResAdd = addInput_("res");
			m_iResMul = addInput_("res[x]", 1.0, Signal::EMul);
			m_oLP = addOutput_("LP");
			m_oHP = addOutput_("HP");
			m_oBP = addOutput_("BP");
			m_oN  = addOutput_("N");
		}

		StateVariableFilter(const StateVariableFilter& a_rhs) :
			StateVariableFilter(a_rhs.getName())
		{}

	protected:

		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override {
			double input_fc = a_inputs.getValue(m_iFcMul)*(getParameter(m_pFc).getDouble() + a_inputs.getValue(m_iFcAdd));
			input_fc = CLAMP(input_fc, 0, 1);
			double fc = LERP(c_minFreq, c_maxFreq, input_fc);
			m_F = 2 * lut_sin.getlinear(0.5*fc / getFs());

			double input_res = a_inputs.getValue(m_iResMul)*(getParameter(m_pRes).getDouble() + a_inputs.getValue(m_iResAdd));
			input_res = CLAMP(input_res, 0, 1);
			double res = LERP(c_minRes, c_maxRes, input_res);
			m_Q = 1.0 / res;

			double input = a_inputs.getValue(0);
			double LPOut=0, HPOut=0, BPOut=0;
			int i = c_oversamplingFactor;
			while (i--) {
				LPOut = m_prevLPOut + m_F*m_prevBPOut;
				HPOut = input - LPOut - m_Q*m_prevBPOut;
				BPOut = m_F*HPOut + m_prevBPOut;

				m_prevBPOut = BPOut;
				m_prevLPOut = LPOut;
			}
			double NOut = HPOut + LPOut;

			a_outputs.setChannel(m_oLP, LPOut);
			a_outputs.setChannel(m_oHP, HPOut);
			a_outputs.setChannel(m_oBP, BPOut);
			a_outputs.setChannel(m_oN, NOut);
		}

	private:
		string _getClassName() const override { return "StateVariableFilter";  };
		Unit* _clone() const override { return new StateVariableFilter(*this); };

	private:
		int m_pFc, m_pRes;
		int m_iResAdd, m_iResMul;
		int m_iFcAdd, m_iFcMul;
		int m_oLP, m_oBP, m_oN, m_oHP;
		double m_prevBPOut, m_prevLPOut;
		double m_F, m_Q;

		const double c_minFreq = 80.0;
		const double c_maxFreq = 6000.0;
		const double c_minRes = 1.0;
		const double c_maxRes = 10.0;
		const int c_oversamplingFactor = 8;
	};
}
#endif

