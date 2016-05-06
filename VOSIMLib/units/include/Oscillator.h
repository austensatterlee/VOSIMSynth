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
#include "DSPMath.h"
#include <vector>

using namespace std;

namespace syn {
	enum WAVE_SHAPE {
		SAW_WAVE = 0,
		SINE_WAVE,
		TRI_WAVE,
		SQUARE_WAVE
	};

	const vector<string> WAVE_SHAPE_NAMES{ "Saw", "Sine", "Tri", "Square" };

	double sampleWaveShape(WAVE_SHAPE shape, double phase, double period, bool useNaive);

	class Oscillator : public Unit {
	public:
		explicit Oscillator(const string& a_name) :
			Unit(a_name),
			m_basePhase(0),
			m_phase(0),
			m_last_phase(0),
			m_phase_step(0),
			m_period(1),
			m_freq(0.0), 
			m_gain(0.0),
			m_bias(0.0),
			m_oOut(addOutput_("out")),
			m_oPhase(addOutput_("ph")),
			m_pGain(addParameter_({ "gain", 0.0, 1.0, 1.0 })),
			m_pPhaseOffset(addParameter_({ "phase", 0.0, 1.0, 0.0 })),
			m_pUnipolar(addParameter_(UnitParameter("unipolar", false))),
			m_iGainMul(addInput_("g[x]",1.0, Signal::EMul)),
			m_iPhaseAdd(addInput_("ph"))
		{
		}

		virtual ~Oscillator() {}

	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;		
		virtual void MSFASTCALL tickPhase_(double a_phaseOffset) GCCFASTCALL;
		virtual void MSFASTCALL updatePhaseStep_() GCCFASTCALL;
		virtual void MSFASTCALL sync_() GCCFASTCALL {};
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
		explicit TunedOscillator(const string& a_name) :
			Oscillator(a_name),
			m_pitch(0),
			m_pTune(addParameter_({ "semi", -12.0, 12.0, 0.0 })),
			m_pOctave(addParameter_({ "oct", -3, 3, 0 }))
		{
			m_iNote = addInput_("pitch");
		}

		explicit TunedOscillator(const TunedOscillator& a_rhs) :
			TunedOscillator(a_rhs.getName())
		{}

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

	class BasicOscillator : public TunedOscillator {
	public:
		explicit BasicOscillator(const string& a_name) :
			TunedOscillator(a_name),
			m_pWaveform(addParameter_(UnitParameter("waveform", WAVE_SHAPE_NAMES)))
		{
		};

		explicit BasicOscillator(const BasicOscillator& a_rhs) : BasicOscillator(a_rhs.getName())
		{}

		virtual ~BasicOscillator() {};
	protected:
		int m_pWaveform;
	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;
	private:
		string _getClassName() const override { return "BasicOscillator"; }

		Unit* _clone() const override { return new BasicOscillator(*this); }
	};

	class LFOOscillator : public Oscillator {	
	public:
		explicit LFOOscillator(const string& a_name) :
			Oscillator(a_name),			
			m_oQuadOut(addOutput_("quad")),
			m_iFreqAdd(addInput_("freq")),
			m_iFreqMul(addInput_("freq[x]",1.0,Signal::EMul)),
			m_iSync(addInput_("sync")),
			m_syncedFreqParam("rate",
			{"1/64","1/32","3/64","1/16","3/32","1/8","1/4","3/8","1/2","3/4","1","3/2","2"},
			{ 1.0/64,1.0/32,3.0/64,1.0/16,3.0/32,1.0/8,1.0/4,3.0/8,1.0/2,3.0/4,1.0,3.0/2,2}, 0),
			m_linFreqParam("freq", 0.0, 30.0, 1.0),
			m_lastSync(0.0)
		{
			m_pWaveform = addParameter_(UnitParameter("waveform", WAVE_SHAPE_NAMES));
			m_pFreq = addParameter_(m_linFreqParam);
			m_pTempoSync = addParameter_(UnitParameter("tempo sync", false));
		};

		explicit LFOOscillator(const BasicOscillator& a_rhs) : LFOOscillator(a_rhs.getName())
		{}

		virtual ~LFOOscillator() {};
	protected:
		int m_oQuadOut;

		int m_pWaveform;
		int m_pFreq;
		int m_pTempoSync;
		int m_iFreqAdd, m_iFreqMul;
		int m_iSync;

		UnitParameter m_syncedFreqParam;
		UnitParameter m_linFreqParam;

		double m_lastSync;
	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;
		void onParamChange_(int a_paramId) override;
	private:
		string _getClassName() const override { return "LFOOscillator"; }

		Unit* _clone() const override { return new LFOOscillator(*this); }
		
	};
};
#endif

