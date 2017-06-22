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

#include "units/OscillatorUnit.h"

namespace syn
{
    class VOSIMLIB_API VosimOscillator : public TunedOscillatorUnit
    {
        DERIVE_UNIT(VosimOscillator)
    public:
        enum Param
        {
            pPulseTune = TunedOscillatorUnit::NUM_PARAMS,
            pNumPulses,
            pPulseDecay,
            NUM_PARAMS
        };

        enum Input
        {
            iPulseTuneAdd = TunedOscillatorUnit::NUM_INPUTS,
            iPulseTuneMul,
            iDecayMul,
            NUM_INPUTS
        };

        explicit VosimOscillator(string name);

        VosimOscillator(const VosimOscillator& a_rhs);

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;
        void MSFASTCALL updatePhaseStep_() GCCFASTCALL override;

    private:
        double m_pulse_step, m_pulse_tune;
        int m_num_pulses;
    };

    class VOSIMLIB_API FormantOscillator : public TunedOscillatorUnit
    {
        DERIVE_UNIT(FormantOscillator)
    public:
        
        enum Param
        {
            pWidth = TunedOscillatorUnit::NUM_PARAMS,
            pFmt,
            NUM_PARAMS
        };

        enum Input
        {
            iWidthAdd = TunedOscillatorUnit::NUM_INPUTS,
            iFmtAdd,
            iWidthMul,
            iFmtMul,
            NUM_INPUTS
        };

        explicit FormantOscillator(string name);

        FormantOscillator(const FormantOscillator& a_rhs);

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;
    };
}
#endif
