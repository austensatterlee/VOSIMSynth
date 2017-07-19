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

/**
* \file units/ADSREnvelope.h
* \brief
* \details
* \author Austen Satterlee
* \date March 6, 2016
*/

#ifndef __ADSRENVELOPE__
#define __ADSRENVELOPE__
#include "vosimlib/Unit.h"

namespace syn {
    class VOSIMLIB_API ADSREnvelope : public Unit {
        DERIVE_UNIT(ADSREnvelope)
    public:
        enum Param {
            pAttack = 0,
            pDecay,
            pSustain,
            pRelease,
            pTimescale
        };

        enum Input {
            iGate = 0,
            iAttack,
            iDecay,
            iSustain,
            iRelease
        };

        explicit ADSREnvelope(const string& name);

        ADSREnvelope(const ADSREnvelope& a_rhs);

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;
        bool isGateRising() const;
        bool isGateFalling() const;
        void onNoteOn_() override;
        void onNoteOff_() override;

    public:
        bool isActive() const override;

        void trigger();
        void release(double a_releaseValue);
        void reset() override;
    private:
        enum EADSRStage {
            Off = 0,
            Attack,
            Decay,
            Sustain,
            Release
        };

        EADSRStage m_currStage;
        double m_phase;
        double m_currInitial;
        double m_currTarget;
        double m_lastOutput;
        double m_lastGate;
    };
}
#endif
