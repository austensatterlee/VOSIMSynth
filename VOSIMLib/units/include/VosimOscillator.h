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

#ifndef __VOSIMOSCILLATOR__
#define __VOSIMOSCILLATOR__

#include "Oscillator.h"

using namespace std;

namespace syn
{
	class VosimOscillator : public TunedOscillator
	{
	public:
		explicit VosimOscillator(string name);

		VosimOscillator(const VosimOscillator& a_rhs);

	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;
		void MSFASTCALL updatePhaseStep_() GCCFASTCALL override;

	private:
		string _getClassName() const override;

		Unit* _clone() const override;

	private:
		double m_pulse_step, m_pulse_tune;
		int m_num_pulses;
		int m_pPulseTune;
		int m_iPulseTuneAdd, m_iPulseTuneMul;
		int m_pNumPulses;
		int m_pPulseDecay;
	};

	class FormantOscillator : public TunedOscillator
	{
	public:

		explicit FormantOscillator(string name);;

		FormantOscillator(const FormantOscillator& a_rhs);

	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;

	private:
		string _getClassName() const override;

		Unit* _clone() const override;

	private:
		int m_pWidth;
		int m_pFmt;

		int m_iWidthAdd;
		int m_iFmtAdd;
		int m_iWidthMul;
		int m_iFmtMul;
	};
}

#endif
