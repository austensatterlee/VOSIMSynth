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
		DERIVE_UNIT(StateVariableFilter)
	public:
		explicit StateVariableFilter(const string& a_name);

		StateVariableFilter(const StateVariableFilter& a_rhs) :
			StateVariableFilter(a_rhs.getName()) {}

	protected:

		void MSFASTCALL process_() GCCFASTCALL override;

	protected:
		int m_pFc, m_pRes;
		int m_iResAdd, m_iResMul;
		int m_iFcAdd, m_iFcMul;
		int m_oLP, m_oBP, m_oN, m_oHP;
		double m_prevBPOut, m_prevLPOut;
		double m_F, m_damp;

		const double c_minRes = 1.0;
		const double c_maxRes = 10.0;
		const int c_oversamplingFactor = 8;
	};

	class TrapStateVariableFilter : public StateVariableFilter
	{
		DERIVE_UNIT(TrapStateVariableFilter)
	public:
		explicit TrapStateVariableFilter(const string& a_name);

		TrapStateVariableFilter(const TrapStateVariableFilter& a_rhs) :
			TrapStateVariableFilter(a_rhs.getName()) {}

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	protected:
		double m_prevInput;
	};
	

	/**
	* 1 Pole Filter (Lag)
	*/
	class OnePoleLP : public Unit
	{
		DERIVE_UNIT(OnePoleLP)
	public:
		explicit OnePoleLP(const string& a_name);

		OnePoleLP(const OnePoleLP& a_rhs) : OnePoleLP(a_rhs.getName()) {};

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		int m_pFc;
		int m_iFcAdd, m_iFcMul;
		double m_state;
	};

	class LadderFilter : public Unit
	{
		DERIVE_UNIT(LadderFilter)
	public:
		explicit LadderFilter(const string& a_name);
		LadderFilter(const LadderFilter& a_rhs) : LadderFilter(a_rhs.getName()) {};
	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
	private:
		int m_pFc;
		int m_iFcAdd, m_iFcMul;
		OnePoleLP m_ladder[4];
	};
}

CEREAL_REGISTER_TYPE(syn::StateVariableFilter)
CEREAL_REGISTER_TYPE(syn::TrapStateVariableFilter)
CEREAL_REGISTER_TYPE(syn::OnePoleLP)
CEREAL_REGISTER_TYPE(syn::LadderFilter)
#endif
