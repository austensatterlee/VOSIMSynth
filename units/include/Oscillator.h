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
                m_basePhase(0),
                m_phase(0),
                m_phase_step(0),
                m_period(1),
                m_freq(1),
                m_pitch(0)
        {
            addInput_("gain");
            addInput_("pitch");
            addOutput_("out");
        }

        virtual ~Oscillator(){}
    protected:
        double m_basePhase;
        double m_phase;
        double m_phase_step;
        double m_period;
        double m_freq;
        double m_pitch;

        int m_pTune;
        int m_pOctave;
        int m_pGain;
    protected:
        virtual void tick_phase();
        virtual void update_step();
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
        virtual void process_(const SignalBus& a_inputs, SignalBus& a_outputs);
    private:
        string _getClassName() const { return "BasicOscillator"; }
        virtual Unit* _clone() const { return new BasicOscillator(*this); }
    };
};
#endif

