#pragma once
#include "SourceUnit.h"

namespace syn {
	enum ADSRStage
	{
		ATTACK = 0,
		DECAY,
		SUSTAIN,
		RELEASE
	};

	class ADSREnvelope :
		public SourceUnit
	{
	public:
		void noteOn(int pitch, int vel) override;
		void noteOff(int pitch, int vel) override;
		int getSamplesPerPeriod() const override;
		bool isActive() const override;
		ADSREnvelope(string name);
		ADSREnvelope(const ADSREnvelope& other);
		virtual ~ADSREnvelope();
	protected:
		virtual void process(int bufind) override;
		UnitParameter& m_attack;
		UnitParameter& m_decay;
		UnitParameter& m_sustain;
		UnitParameter& m_release;
	private:
		double m_phase;
		ADSRStage m_currStage;
		double m_initial;
		double m_target;
		bool m_isActive;
		virtual Unit* cloneImpl() const override { return new ADSREnvelope(*this); };
		virtual string getClassName() const override { return "ADSREnvelope"; }; 
		virtual void onSampleRateChange(double newfs) override;
	};
}