#ifndef __VosimOscillator__
#define __VosimOscillator__

#include "Oscillator.h"

using namespace std;

namespace syn {
    class VosimOscillator : public Oscillator {
    public:
        VosimOscillator(string name) :
                Oscillator(name),
                m_pPulseTune(addParameter_({"pulse freq", 0.0, 1.0, 0.5})),
                m_pNumPulses(addParameter_({"number", 1, 8, 1})),
                m_pPulseDecay(addParameter_({"decay", 0.0, 1.0, 0.0})),
                m_pulse_step(0.0),
                m_curr_pulse_num(0),
                m_curr_pulse_gain(1.0)
        { }

        VosimOscillator(const VosimOscillator& a_rhs) :
                VosimOscillator(a_rhs.getName())
        { }

    protected:
        void process_(const SignalBus& a_inputs, SignalBus& a_outputs);

        void update_step() override;

    private:
        string _getClassName() const override
        { return "VosimOscillator"; }

        virtual Unit* _clone() const
        { return new VosimOscillator(*this); }

    private:
        double m_pulse_step;
        int m_curr_pulse_num;
        double m_curr_pulse_gain;
        int m_pPulseTune;
        int m_pNumPulses;
        int m_pPulseDecay;
    };

    class FormantOscillator : public Oscillator {
    public:

        FormantOscillator(string name) :
                Oscillator(name),
                m_pWidth(addParameter_({"width", 0.0, 1.0, 0.5})),
                m_pFmtpitch(addParameter_({"fmtpitch", 0.0, 1.0, 0.5}))
        {
        };

        FormantOscillator(const FormantOscillator& a_rhs) :
                FormantOscillator(a_rhs.getName())
        { }

    protected:
        void process_(const SignalBus& a_inputs, SignalBus& a_outputs);

    private:
        string _getClassName() const override
        { return "FormantOscillator"; }

        virtual Unit* _clone() const
        { return new FormantOscillator(*this); }

    private:
        int m_pWidth;
        int m_pFmtpitch;
    };
}

#endif // __VosimOscillator__


