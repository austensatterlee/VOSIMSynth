#ifndef __VosimOscillator__
#define __VosimOscillator__

#include "Oscillator.h"

using namespace std;

namespace syn {
    class VosimOscillator : public TunedOscillator {
    public:
        VosimOscillator(string name) :
			TunedOscillator(name),
                m_pulse_step(0.0),
				m_pulse_tune(0),
				m_num_pulses(1),
                m_pPulseTune(addParameter_({"pulse freq", 0.0, 1.0, 0.0})),
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
        void updatePhaseStep_() override;

    private:
        string _getClassName() const override
        { return "VosimOscillator"; }

	    Unit* _clone() const override { return new VosimOscillator(*this); }

    private:
        double m_pulse_step, m_pulse_tune;
		int m_num_pulses;
        int m_pPulseTune, m_iPulseTune;
        int m_pNumPulses;
        int m_pPulseDecay;
    };

    class FormantOscillator : public TunedOscillator {
    public:

        FormantOscillator(string name) :
			TunedOscillator(name),
                m_pWidth(addParameter_({"width", 0.0, 1.0, 0.0})),
                m_pFmtpitch(addParameter_({"formant", 0.0, 1.0, 0.0}))
        {
			m_iWidth = addInput_("width");
			m_iFmtpitch = addInput_("formant");
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

		int m_iWidth;
		int m_iFmtpitch;
    };
}

#endif // __VosimOscillator__


