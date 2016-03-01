#ifndef __OSCILLATOR__
#define __OSCILLATOR__

#include "Unit.h"
#include <vector>

using namespace std;

namespace syn {
	enum WAVE_SHAPE {
		SAW_WAVE = 0,
		NAIVE_SAW_WAVE,
		SINE_WAVE,
		TRI_WAVE,
		SQUARE_WAVE,
		NAIVE_SQUARE_WAVE
	};

	const vector<string> WAVE_SHAPE_NAMES{ "Saw", "Naive saw", "Sine", "Tri", "Square", "Naive square" };

	double sampleWaveShape(WAVE_SHAPE shape, double phase, double period);

	class Oscillator : public Unit {
	public:
		Oscillator(const string& a_name) :
			Unit(a_name),
			m_basePhase(0),
			m_phase(0),
			m_last_phase(0),
			m_phase_step(0),
			m_period(1),
			m_freq(0.0), m_pGain(addParameter_({ "gain", 0.0, 1.0, 1.0 })),
			m_pPhaseOffset(addParameter_({ "phase", 0.0, 1.0, 0.0 }))
		{
			m_iGainAdd = addInput_("g[x]",1.0, Signal::EMul);
			m_iPhaseAdd = addInput_("ph[+]");
			m_oOut = addOutput_("out");
			m_oPhase = addOutput_("ph");
		}

		virtual ~Oscillator() {}

	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
		virtual void tickPhase_(double a_phaseOffset);
		virtual void updatePhaseStep_();
		virtual void sync_() {};
	protected:
		double m_basePhase;
		double m_phase, m_last_phase;
		double m_phase_step;
		double m_period;
		double m_freq;
		double m_gain;

		int m_oOut;
		int m_oPhase;
	private:
		int m_pGain;
		int m_pPhaseOffset;

		int m_iGainAdd;
		int m_iPhaseAdd;
	};

	class TunedOscillator : public Oscillator
	{
	public:
		TunedOscillator(const string& a_name) :
			Oscillator(a_name),
			m_pitch(0),
			m_pTune(addParameter_({ "semi", -12.0, 12.0, 0.0 })),
			m_pOctave(addParameter_({ "oct", -3, 3, 0 }))
		{
			m_iNote = addInput_("pitch");
		}

		TunedOscillator(const TunedOscillator& a_rhs) :
			TunedOscillator(a_rhs.getName())
		{}

	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
		void updatePhaseStep_() override;
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
		BasicOscillator(const string& a_name) :
			TunedOscillator(a_name),
			m_pWaveform(addParameter_(UnitParameter("waveform", WAVE_SHAPE_NAMES)))
		{
		};

		BasicOscillator(const BasicOscillator& a_rhs) : BasicOscillator(a_rhs.getName())
		{}

		virtual ~BasicOscillator() {};
	protected:
		int m_pWaveform;
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
	private:
		string _getClassName() const override { return "BasicOscillator"; }

		Unit* _clone() const override { return new BasicOscillator(*this); }
	};

	class LFOOscillator : public Oscillator {	
	public:
		LFOOscillator(const string& a_name) :
			Oscillator(a_name),
			m_pWaveform(addParameter_(UnitParameter("waveform", WAVE_SHAPE_NAMES))),
			m_pTempoSync(addParameter_(UnitParameter("tempo sync", false))),
			m_pFreq(addParameter_(UnitParameter("freq",0.0,20.0,1.0))),			
			m_lastSync(0.0)
		{
			m_iFreqAdd = addInput_("freq[+]");
			m_iFreqMul = addInput_("freq[x]",1.0,Signal::EMul);
			m_iSync = addInput_("sync");
		};

		LFOOscillator(const BasicOscillator& a_rhs) : LFOOscillator(a_rhs.getName())
		{}

		virtual ~LFOOscillator() {};
	protected:
		int m_pWaveform;
		int m_pFreq;
		int m_pTempoSync;
		int m_iFreqAdd, m_iFreqMul;
		int m_iSync;

		double m_lastSync;
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
		void onParamChange_(int a_paramId) override;
	private:
		string _getClassName() const override { return "LFOOscillator"; }

		Unit* _clone() const override { return new LFOOscillator(*this); }
		
	};
};
#endif

