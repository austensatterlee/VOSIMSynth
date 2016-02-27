#pragma once

#include "Unit.h"

namespace syn {
    class ADSREnvelope : public Unit {
    public:
        ADSREnvelope(const string& name);

        ADSREnvelope(const ADSREnvelope& a_rhs);

    protected:
	    void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;

    public:
        virtual bool isActive() const override;

	    void trigger();
	    void release();
    private:
	    string _getClassName() const override { return "ADSREnvelope"; };

	    Unit* _clone() const override { return new ADSREnvelope(*this); }

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
		int m_pTimeScale;

		int m_iGate;
    };
}