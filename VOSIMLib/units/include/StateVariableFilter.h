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

using namespace std;

namespace syn
{
	class StateVariableFilter : public Unit
	{
	public:
		explicit StateVariableFilter(const string& a_name);

		StateVariableFilter(const StateVariableFilter& a_rhs) :
			StateVariableFilter(a_rhs.getName()) {}

	protected:

		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override {
			return "StateVariableFilter";
		};

		Unit* _clone() const override {
			return new StateVariableFilter(*this);
		};

	private:
		int m_pFc, m_pRes;
		int m_iResAdd, m_iResMul;
		int m_iFcAdd, m_iFcMul;
		int m_oLP, m_oBP, m_oN, m_oHP;
		double m_prevBPOut, m_prevLPOut;
		double m_F, m_Q;

		const double c_minRes = 1.0;
		const double c_maxRes = 10.0;
		const int c_oversamplingFactor = 8;
	};
}
#endif
