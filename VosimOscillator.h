#ifndef __VosimOscillator__
#define __VosimOscillator__

#include "Oscillator.h"
#include "SourceUnit.h"
#include "RandomOscillator.h"
#include <random>
#include <cmath>

using namespace std;

namespace syn
{
	class VosimOscillator : public Oscillator
	{
	public:
		VosimOscillator(string name) :
			Oscillator(name),
			m_relativeamt(addDoubleParam("relative", 0, 1, 0.5, 1e-3)),
			m_decay(addDoubleParam("decay", 0, 1, 0.9, 1e-3, 2.0)),
			m_ppitch(addDoubleParam("pulsepitch", 0, 1, 0.5, 1e-3, 2.0)),
			m_number(addIntParam("number", 0, 16, 1)),
			m_curr_pulse_gain(1.0),
			m_pulse_step(0.0),
			m_pulse_phase(0.0), m_last_pulse_phase(0),
			m_unwrapped_pulse_phase(0.0)
		{
		};

		VosimOscillator(const VosimOscillator& vosc);
		virtual void process(int bufind) override;
		virtual void sync() override;
		UnitParameter& m_relativeamt;
		UnitParameter& m_decay;
		UnitParameter& m_ppitch;
		UnitParameter& m_number;
	private:
		/* internal state */
		virtual Unit* cloneImpl() const override
		{
			return new VosimOscillator(*this);
		}

		virtual inline string getClassName() const override
		{
			return "VosimOscillator";
		}

		double m_curr_pulse_gain;
		double m_pulse_step;
		double m_pulse_phase;
		double m_last_pulse_phase;
		double m_unwrapped_pulse_phase;
	};
}

#endif // __VosimOscillator__


