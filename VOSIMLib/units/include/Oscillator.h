#ifndef __OSCILLATOR__
#define __OSCILLATOR__

#include "Unit.h"
#include <cstdint>
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

    const vector<string> WAVE_SHAPE_NAMES{"Saw", "Naive saw", "Sine", "Tri", "Square", "Naive square"};

    double sampleWaveShape(WAVE_SHAPE shape, double phase, double period);

    class Oscillator : public Unit {
    public:
        Oscillator(const string& a_name) :
                Unit(a_name),
                m_pTune(addParameter_({"tune", -12.0, 12.0, 0.0})),
                m_pOctave(addParameter_({"octave", -2, 2, 0})),
                m_pGain(addParameter_({"gain", 0.0, 1.0, 1.0})),
				m_pPhaseOffset(addParameter_({"phase", 0.0, 0.5, 0.0})),
				m_iGain(addInput_("gain")),
				m_iNote(addInput_("note")),
				m_iPhaseOffset(addInput_("phase")),
                m_basePhase(0),
                m_phase(0), m_last_phase(0),
                m_phase_step(0),
                m_period(1),
                m_freq(1),
                m_pitch(0)
        {
            addOutput_("out");
        }

        virtual ~Oscillator(){}
    protected:
        double m_basePhase;
        double m_phase, m_last_phase;
        double m_phase_step;
        double m_period;
        double m_freq;
        double m_pitch;
		double m_gain;
    protected:
	    void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
        virtual void tickPhase_(double a_phaseOffset);
        virtual void updatePhaseStep_(const SignalBus& a_inputs, SignalBus& a_outputs);
	    virtual void sync_() {};
		void onNoteOn_() override;
	private:
		int m_pTune;
		int m_pOctave;
		int m_pGain;
		int m_pPhaseOffset;

		int m_iGain;
		int m_iNote;
		int m_iPhaseOffset;
    };

    class BasicOscillator : public Oscillator {
    public:
        BasicOscillator(const string& a_name) :
                Oscillator(a_name),
                m_pWaveform(addParameter_(UnitParameter("waveform", WAVE_SHAPE_NAMES)))
        {
        };

        BasicOscillator(const BasicOscillator& a_rhs) : BasicOscillator(a_rhs.getName())
        {}

        virtual ~BasicOscillator(){};
    protected:
        int m_pWaveform;
    protected:
	    void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
    private:
        string _getClassName() const override { return "BasicOscillator"; }

	    Unit* _clone() const override { return new BasicOscillator(*this); }
    };

	class LFOOscillator : public BasicOscillator {
	public:
		LFOOscillator(const string& a_name) :
			BasicOscillator(a_name),
			m_pWaveform(addParameter_(UnitParameter("waveform", WAVE_SHAPE_NAMES)))
		{
		};

		LFOOscillator(const BasicOscillator& a_rhs) : LFOOscillator(a_rhs.getName())
		{}

		virtual ~LFOOscillator() {};
	protected:
		int m_pWaveform;
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
	private:
		string _getClassName() const override { return "LFOOscillator"; }

		Unit* _clone() const override { return new LFOOscillator(*this); }
	};
};
#endif

