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
		DERIVE_UNIT(VosimOscillator)
	public:
		enum Param
		{
			pPulseTune = TunedOscillator::NumParams,
			pNumPulses,
			pPulseDecay,
			NumParams
		};

		enum Input
		{
			iPulseTuneAdd = TunedOscillator::NumInputs,
			iPulseTuneMul,
			iDecayMul,
			NumInputs
		};

		explicit VosimOscillator(string name);

		VosimOscillator(const VosimOscillator& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
		void MSFASTCALL updatePhaseStep_() GCCFASTCALL override;

	private:
		double m_pulse_step, m_pulse_tune;
		int m_num_pulses;
	};

	class FormantOscillator : public TunedOscillator
	{
		DERIVE_UNIT(FormantOscillator)
	public:
		
		enum Param
		{
			pWidth = TunedOscillator::NumParams,
			pFmt,
			NumParams
		};

		enum Input
		{
			iWidthAdd = TunedOscillator::NumInputs,
			iFmtAdd,
			iWidthMul,
			iFmtMul,
			NumInputs
		};

		explicit FormantOscillator(string name);

		FormantOscillator(const FormantOscillator& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
	};
}

CEREAL_REGISTER_TYPE(syn::VosimOscillator)
CEREAL_REGISTER_TYPE(syn::FormantOscillator)

#endif
