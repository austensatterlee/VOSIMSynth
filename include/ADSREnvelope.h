#pragma once
#include "Unit.h"
#define getFs() 100.0

namespace syn {
	class ADSREnvelope : public Cloneable<Unit,ADSREnvelope>
	{
	public:
        ADSREnvelope(const string& name);
        void trigger();
        void release();

    protected:
        virtual bool onParamChange_(int a_paramId) override;
        virtual void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
    private:
        virtual string _getClassName() const override { return "ADSREnvelope"; };

	private:
        enum EADSRStage
        {
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