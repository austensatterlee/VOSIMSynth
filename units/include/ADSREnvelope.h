#pragma once

#include "Unit.h"

namespace syn {
    class ADSREnvelope : public Unit {
    public:
        ADSREnvelope(const string& name);

        ADSREnvelope(const ADSREnvelope& a_rhs);

    protected:
        virtual void process_(const SignalBus& a_inputs, SignalBus& a_outputs);

    public:
        virtual bool isActive() const override;

        virtual void onNoteOn_();
        virtual void onNoteOff_();
    private:
        virtual string _getClassName() const
        { return "ADSREnvelope"; };

        virtual Unit* _clone() const
        { return new ADSREnvelope(*this); }

    private:
        enum EADSRStage {
            Attack = 0,
            Decay,
            Sustain,
            Release
        };
        double m_phase;
        EADSRStage m_currStage;
        double m_initial;
        double m_target;
        bool m_isActive;
        int m_pAttack;
        int m_pDecay;
        int m_pSustain;
        int m_pRelease;
    };
}