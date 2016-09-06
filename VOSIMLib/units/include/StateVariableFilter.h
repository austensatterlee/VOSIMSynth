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

namespace syn
{
	class StateVariableFilter : public Unit
	{
		DERIVE_UNIT(StateVariableFilter)
	public:

		const int c_oversamplingFactor = 8;

		explicit StateVariableFilter(const string& a_name);

		StateVariableFilter(const StateVariableFilter& a_rhs) :
			StateVariableFilter(a_rhs.getName()) {}

		virtual void reset() override;

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
	};

	class TrapStateVariableFilter : public StateVariableFilter
	{
		DERIVE_UNIT(TrapStateVariableFilter)
	public:
		explicit TrapStateVariableFilter(const string& a_name);

		TrapStateVariableFilter(const TrapStateVariableFilter& a_rhs) :
			TrapStateVariableFilter(a_rhs.getName()) {}

		virtual void reset() override;
	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	protected:
		double m_prevInput;
	};

	/**
	 * 1 Pole "TPT" implementation
	 */
	struct _OnePoleLP
	{
		/**
		 * Set forward gain by specifying cutoff frequency and sampling rate.
		 */
		void setFc(double a_fc, double a_fs);

		double process(double a_input);

		void reset();

		double m_state = 0.0;
		double m_G = 0.0; /// feed forward gain
	};
	

	/**
	 * 1 Pole "TPT" unit wrapper
	 */
	class OnePoleLP : public Unit
	{
		DERIVE_UNIT(OnePoleLP)
	public:
		enum Param
		{
			pFc = 0
		};

		enum Input
		{
			iAudioIn = 0,
			iFcAdd,
			iFcMul
		};

		enum Output
		{
			oLP = 0,
			oHP = 1
		};

		explicit OnePoleLP(const string& a_name);

		OnePoleLP(const OnePoleLP& a_rhs) : OnePoleLP(a_rhs.getName()) {};

		double getState() const;

		virtual void reset() override;;

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
	private:
		_OnePoleLP implem;
	};

	struct LadderFilterBase : public Unit
	{
    public:
      explicit LadderFilterBase(const string& a_name);

      const int c_oversamplingFactor = 8;
      const double VT = 0.312;

      enum Param
      {
        pFc = 0,
        pFb,
        pDrv
      };

      enum Input
      {
        iAudioIn = 0,
        iFcAdd,
        iFcMul
      };
	};

	class LadderFilter : public LadderFilterBase
	{
		DERIVE_UNIT(LadderFilter)
	public:
		explicit LadderFilter(const string& a_name);
		LadderFilter(const LadderFilter& a_rhs) : LadderFilter(a_rhs.getName()) {};
		void reset() override;
	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
	protected:
		array<double, 4> m_V;
		array<double, 4> m_dV;
		array<double, 4> m_tV;
	};

	class LadderFilterTwo : public LadderFilterBase
	{
		DERIVE_UNIT(LadderFilterTwo)
	public:
		explicit LadderFilterTwo(const string& a_name);
		LadderFilterTwo(const LadderFilterTwo& a_rhs) : LadderFilterTwo(a_rhs.getName()) {};
		void reset() override;
	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
	protected:
		typedef Eigen::Matrix<double, 5, 1> Vector5d;
		_OnePoleLP m_LP[4];
		Vector5d m_ffGains; /// state feed forward gains
	};
}
#endif
