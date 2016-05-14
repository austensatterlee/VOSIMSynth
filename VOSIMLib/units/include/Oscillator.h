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

using namespace std;

namespace syn
{
	enum WAVE_SHAPE
	{
		SAW_WAVE = 0,
		SINE_WAVE,
		TRI_WAVE,
		SQUARE_WAVE
	};

	const vector<string> WAVE_SHAPE_NAMES{"Saw", "Sine", "Tri", "Square"};

	double sampleWaveShape(WAVE_SHAPE shape, double phase, double period, bool useNaive);

	class Oscillator : public Unit
	{
	public:
		explicit Oscillator(const string& a_name);

	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;
		virtual void MSFASTCALL tickPhase_(double a_phaseOffset) GCCFASTCALL;
		virtual void MSFASTCALL updatePhaseStep_() GCCFASTCALL;

		virtual void MSFASTCALL sync_() GCCFASTCALL ;;

	protected:
		double m_basePhase;
		double m_phase, m_last_phase;
		double m_phase_step;
		double m_period;
		double m_freq;
		double m_gain;
		double m_bias;

		int m_oOut;
		int m_oPhase;
	private:
		int m_pGain;
		int m_pPhaseOffset;
		int m_pUnipolar;

		int m_iGainMul;
		int m_iPhaseAdd;
	};

	class TunedOscillator : public Oscillator
	{
	public:
		explicit TunedOscillator(const string& a_name);

		explicit TunedOscillator(const TunedOscillator& a_rhs);

	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) override GCCFASTCALL;
		void MSFASTCALL updatePhaseStep_() override GCCFASTCALL;
		void onNoteOn_() override;
	protected:
		double m_pitch;
	private:
		int m_pTune;
		int m_pOctave;

		int m_iNote;
	};

	class BasicOscillator : public TunedOscillator
	{
	public:
		explicit BasicOscillator(const string& a_name);;

		explicit BasicOscillator(const BasicOscillator& a_rhs);

	protected:
		int m_pWaveform;
	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;
	private:
		string _getClassName() const override;

		Unit* _clone() const override;
	};

	class LFOOscillator : public Oscillator
	{
	public:
		explicit LFOOscillator(const string& a_name);

		explicit LFOOscillator(const BasicOscillator& a_rhs) : LFOOscillator(a_rhs.getName()) {}

		virtual ~LFOOscillator();

	protected:
		int m_oQuadOut;

		int m_pWaveform;
		int m_pFreq;
		int m_pTempoSync;
		int m_iFreqAdd, m_iFreqMul;
		int m_iSync;

		UnitParameter* m_syncedFreqParam;
		UnitParameter* m_linFreqParam;

		double m_lastSync;
	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;
		void onParamChange_(int a_paramId) override;
	private:
		string _getClassName() const override;

		Unit* _clone() const override;
	};
};
#endif
