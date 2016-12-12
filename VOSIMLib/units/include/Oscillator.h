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

#ifndef __OSCILLATOR__
#define __OSCILLATOR__

#include "Unit.h"

namespace syn
{
	enum WAVE_SHAPE
	{
		SAW_WAVE = 0,
		SINE_WAVE,
		TRI_WAVE,
		SQUARE_WAVE
	};

	const vector<string> WAVE_SHAPE_NAMES{ "Saw", "Sine", "Tri", "Square" };

	class VOSIMLIB_API Oscillator : public Unit
	{
	public:
		enum Param
		{
			pGain = 0,
			pPhaseOffset,
			pUnipolar,
			NumParams
		};

		enum Output
		{
			oOut = 0,
			oPhase,
			NumOutputs
		};

		enum Input
		{
			iGainMul = 0,
			iPhaseAdd,
			NumInputs
		};

		explicit Oscillator(const string& a_name);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
		virtual void MSFASTCALL tickPhase_(double a_phaseOffset) GCCFASTCALL;
		virtual void MSFASTCALL updatePhaseStep_() GCCFASTCALL;

		virtual void MSFASTCALL sync_() GCCFASTCALL;

	protected:
		double m_basePhase;
		double m_phase, m_last_phase;
		double m_phase_step;
		double m_period;
		double m_freq;
		double m_gain;
		double m_bias;
	};

	class VOSIMLIB_API TunedOscillator : public Oscillator
	{
	public:
		enum Param
		{
			pTune = Oscillator::NumParams,
			pOctave,
			NumParams
		};

		enum Input
		{
			iNote = Oscillator::NumInputs,
			NumInputs
		};

		explicit TunedOscillator(const string& a_name);

		explicit TunedOscillator(const TunedOscillator& a_rhs);

	protected:
		void MSFASTCALL process_() override GCCFASTCALL;
		void MSFASTCALL updatePhaseStep_() override GCCFASTCALL;
		void onNoteOn_() override;
	protected:
		double m_pitch;
	};

	class VOSIMLIB_API BasicOscillator : public TunedOscillator
	{
		DERIVE_UNIT(BasicOscillator)
	public:
		enum Param
		{
			pWaveform = TunedOscillator::NumParams
		};
		explicit BasicOscillator(const string& a_name);

		explicit BasicOscillator(const BasicOscillator& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
	};

	class VOSIMLIB_API LFOOscillator : public Oscillator
	{
		DERIVE_UNIT(LFOOscillator)
	public:
		enum Param
		{
			pWaveform = Oscillator::NumParams,
			pBPMFreq,
			pFreq,
			pTempoSync,
			NumParams
		};

		enum Input
		{
			iFreqAdd = Oscillator::NumInputs,
			iFreqMul,
			iSync
		};

		enum Output
		{
			oQuadOut = Oscillator::NumOutputs
		};

		explicit LFOOscillator(const string& a_name);

		explicit LFOOscillator(const LFOOscillator& a_rhs);

		explicit LFOOscillator(const BasicOscillator& a_rhs) : LFOOscillator(a_rhs.name()) {}

	protected:

		double m_lastSync;
	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
		void onParamChange_(int a_paramId) override;
	};
}
#endif
