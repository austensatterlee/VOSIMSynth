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

#pragma once
#include "vosimlib/Unit.h"

namespace syn {
    class VOSIMLIB_API ADSREnvelope : public Unit {
        DERIVE_UNIT(ADSREnvelope)
    public:
        enum Param {
            pAtkTime = 0,
            pDecTime,
            pSustain,
            pRelTime,
            pTimescale,
            pUpShape,
            pDownShape,
            pLegato
        };

        enum Input {
            iGate = 0
        };

        explicit ADSREnvelope(const string& a_name);

        ADSREnvelope(const ADSREnvelope& a_rhs);

    public:
        bool isActive() const override;
        void reset() override;

    protected:
        void process_() override;
        void onNoteOn_() override;
        void onNoteOff_() override;
        void onParamChange_(int a_paramId) override;
        void onFsChange_() override;
        void setAttackTime_(double a_time);
        void setDecayTime_(double a_time);
        void setReleaseTime_(double a_time);
        void setTargetRatioUp_(double a_targetRatio);
        void setTargetRatioDown_(double a_targetRatio);
        double calcFb_(double a_time, double a_targetRatio);

    private:
        enum State {
            Off = 0,
            Attack,
            Decay,
            Sustain,
            Release,
            Retrigger
        };

        double m_sustain;
        double m_timeScale;
        double m_atkTime, m_atkBias, m_atkFb;
        double m_decTime, m_decBias, m_decFb;
        double m_relTime, m_relBias, m_relFb;
        double m_targetRatioUp, m_targetRatioDown;
        bool m_legato;

        State m_currState;
        double m_lastOutput;
        bool m_lastGate;
    };
}