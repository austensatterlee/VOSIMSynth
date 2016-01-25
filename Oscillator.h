#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include <cstdint>
#include <cmath>
#include <vector>
#include "SourceUnit.h"

using namespace std;

namespace syn
{
	enum WAVE_SHAPE
	{
		SAW_WAVE = 0,
		NAIVE_SAW_WAVE,
		SINE_WAVE,
		NAIVE_SINE_WAVE,
		TRI_WAVE,
		SQUARE_WAVE,
		NAIVE_SQUARE_WAVE,
		NUM_OSC_MODES
	};

	const vector<string> WAVE_SHAPE_NAMES{"Saw","Naive saw","Sine","Naive sine", "Tri","Square","Naive square"};

	double sampleWaveShape(WAVE_SHAPE shape, double phase, double period);

	class Oscillator : public SourceUnit
	{
	public:
		Oscillator(string name) : SourceUnit(name),
		                          m_gain(addDoubleParam("gain", 0.0, 1.0, 0.1, 1e-2)),
		                          m_phase_shift(addDoubleParam("phase", 0.0, 0.5, 0.0, 1e-2)),
		                          m_basePhase(0),
		                          m_phase(0),
		                          m_phase_step(0),
		                          m_period(1),
		                          m_freq(1),
		                          isFreqDirty(true) { };

		Oscillator(const Oscillator& osc) :
			Oscillator(osc.m_name) {
			m_basePhase = osc.m_basePhase;
			m_phase = osc.m_phase;
			m_phase_step = osc.m_phase_step;
			m_period = osc.m_period;
			m_freq = osc.m_freq;
		}

		int getSamplesPerPeriod() const override {
			return static_cast<int>(m_period);
		}

		virtual void sync() {
			m_basePhase = 0;
		};

		bool isActive() const override {
			return m_gain != 0;
		};

		UnitParameter& m_gain;
		UnitParameter& m_phase_shift;
	protected:
		double m_basePhase;
		double m_phase;
		double m_phase_step;
		double m_period;
		double m_freq;

		virtual void tick_phase();
		virtual void update_step();
		void onSampleRateChange(double newfs) override;
	private:
		void beginProcessing() override;
		void finishProcessing() override;
		bool isFreqDirty;
	};

	class BasicOscillator : public Oscillator
	{
	public:

		BasicOscillator(string name) : Oscillator(name),
		                               m_pitch(0), m_velocity(0), m_isPitchDirty(false),
		                               m_waveform(addEnumParam("waveform", WAVE_SHAPE_NAMES, 0)),
		                               m_tune(addDoubleParam("tune", -12.0, 12.0, 0.0, 1e-2)),
		                               m_octave(addIntParam("octave", -3, 3, 0)) {};

		BasicOscillator(const BasicOscillator& other) : BasicOscillator(other.m_name) {};

		void noteOn(int pitch, int vel) override;
		void noteOff(int pitch, int vel) override;

		UnitParameter& m_waveform;
		UnitParameter& m_tune;
		UnitParameter& m_octave;
	protected:
		void update_step() override;
		void process(int bufind) override;
		double m_pitch;
		bool m_isPitchDirty;
		double m_velocity;
	private:
		Unit* cloneImpl() const override {
			return new BasicOscillator(*this);
		};

		string getClassName() const override {
			return "BasicOscillator";
		};
	};

	class LFOOscillator : public Oscillator
	{
	public:

		LFOOscillator(string name) : Oscillator(name),
		                             m_rate(addIntParam("rate", 1, 16, 1)),
		                             m_freq_param(addDoubleParam("freq", 1e-1, 20.0, 1.0, 1e-1)),
		                             m_waveform(addEnumParam("waveform", WAVE_SHAPE_NAMES, 0)),
		                             m_reset(addBoolParam("Reset", 0)),
		                             m_useBPM(addBoolParam("BPM", false)),
		                             m_tempo(120) { }

		LFOOscillator(const LFOOscillator& other) : LFOOscillator(other.m_name) {}

		void noteOn(int pitch, int vel) override {
			if (m_reset)
				sync();
		}

		void noteOff(int pitch, int vel) override { }

		void onTempoChange(double newtempo) override;


		UnitParameter& m_rate;
		UnitParameter& m_freq_param;
		UnitParameter& m_waveform;
		UnitParameter& m_reset;
		UnitParameter& m_useBPM;
	protected:
		virtual void process(int bufind) override;
		void update_step() override;
		double m_tempo;
	private:
		Unit* cloneImpl() const override {
			return new LFOOscillator(*this);
		};

		string getClassName() const override {
			return "LFOOscillator";
		};
	};
};
#endif

