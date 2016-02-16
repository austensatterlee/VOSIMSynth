#ifndef __VosimOscillator__
#define __VosimOscillator__

#include "Oscillator.h"

using namespace std;

namespace syn {
    class VosimOscillator : public Oscillator {
    public:
        VosimOscillator(string name) :
                Oscillator(name),
                m_pulse_step(0.0),
				m_num_pulses(1),
                m_pPulseTune(addParameter_({"pulse freq", 0.0, 1.0, 0.5})),
                m_pNumPulses(addParameter_({"number", 1, 8, 1})),
                m_pPulseDecay(addParameter_({"decay", 0.0, 1.0, 0.0})) 
		{
			m_iPulseTune = addInput_("pulse tune");
        }

        VosimOscillator(const VosimOscillator& a_rhs) :
                VosimOscillator(a_rhs.getName())
        { }

    protected:
	    void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
        void updatePhaseStep_(const SignalBus& a_inputs, SignalBus& a_outputs) override;

    private:
        string _getClassName() const override
        { return "VosimOscillator"; }

	    Unit* _clone() const override { return new VosimOscillator(*this); }

    private:
        double m_pulse_step;
		int m_num_pulses;
        int m_pPulseTune, m_iPulseTune;
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
	    void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;

    private:
        string _getClassName() const override
        { return "FormantOscillator"; }

	    Unit* _clone() const override { return new FormantOscillator(*this); }

    private:
        int m_pWidth;
        int m_pFmtpitch;
    };
}

#endif // __VosimOscillator__


