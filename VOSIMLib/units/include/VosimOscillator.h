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

#ifndef __VOSIMOSCILLATOR__
#define __VOSIMOSCILLATOR__

#include "Oscillator.h"

using namespace std;

namespace syn {
    class VosimOscillator : public TunedOscillator {
    public:
        explicit VosimOscillator(string name) :
			TunedOscillator(name),
                m_pulse_step(0.0),
				m_pulse_tune(0),
				m_num_pulses(1),
                m_pPulseTune(addParameter_({"fp", 0.0, 1.0, 0.0})),
                m_pNumPulses(addParameter_({"num", 1, 8, 1})),
                m_pPulseDecay(addParameter_({"dec", 0.0, 1.0, 0.0})) 
		{
			m_iPulseTuneAdd = addInput_("fp");
			m_iPulseTuneMul = addInput_("fp[x]",1.0,Signal::EMul);
        }

        VosimOscillator(const VosimOscillator& a_rhs) :
                VosimOscillator(a_rhs.getName())
        { }

    protected:
	    void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;
        void MSFASTCALL updatePhaseStep_() GCCFASTCALL override;

    private:
        string _getClassName() const override
        { return "VosimOscillator"; }

	    Unit* _clone() const override { return new VosimOscillator(*this); }

    private:
        double m_pulse_step, m_pulse_tune;
		int m_num_pulses;
		int m_pPulseTune;
		int m_iPulseTuneAdd, m_iPulseTuneMul;
        int m_pNumPulses;
        int m_pPulseDecay;
    };

    class FormantOscillator : public TunedOscillator {
    public:

        explicit FormantOscillator(string name) :
			TunedOscillator(name),
                m_pWidth(addParameter_({"width", 0.0, 1.0, 0.0})),
                m_pFmt(addParameter_({"fmt", 0.0, 1.0, 0.0}))
        {
			m_iWidthAdd = addInput_("w");
			m_iWidthMul = addInput_("w[x]",1.0, Signal::EMul);
			m_iFmtAdd = addInput_("fmt");
			m_iFmtMul = addInput_("fmt[x]",1.0, Signal::EMul);
        };

        FormantOscillator(const FormantOscillator& a_rhs) :
                FormantOscillator(a_rhs.getName())
        { }

    protected:
	    void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;

    private:
        string _getClassName() const override
        { return "FormantOscillator"; }

	    Unit* _clone() const override { return new FormantOscillator(*this); }

    private:
        int m_pWidth;
        int m_pFmt;

		int m_iWidthAdd;
		int m_iFmtAdd;
		int m_iWidthMul;
		int m_iFmtMul;
    };
}

#endif


