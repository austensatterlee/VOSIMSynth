#ifndef __VosimOscillator__
#define __VosimOscillator__

#include "Oscillator.h"
#include "SourceUnit.h"
#include <random>

using namespace std;

namespace syn
{
	class VosimOscillator : public BasicOscillator
	{
	
	public:

		VosimOscillator(string name) :
			BasicOscillator(name),
			m_pulse_tune(addDoubleParam("pulse freq", 0.0, 1.0, 0.5, 1e-3)),
			m_num_pulses(addIntParam("number", 1, 8, 1)),
			m_pulse_decay(addDoubleParam("decay", 0.0, 1.0, 0.0, 1e-3)),
			m_pulse_step(0.0),
			m_curr_pulse_num(0),
			m_curr_pulse_gain(1.0)
		{
			m_waveform.setCanEdit(false);
			m_waveform.setCanModulate(false);
		};

		VosimOscillator(const VosimOscillator& vosc);
		void onParamChange(const UnitParameter* param) override;
		void process(int bufind) override;
		void sync() override;

		UnitParameter& m_pulse_tune;
		UnitParameter& m_num_pulses;
		UnitParameter& m_pulse_decay;
	protected:
			void update_step() override;
	private:
		Unit* cloneImpl() const override
		{
			return new VosimOscillator(*this);
		}

		string getClassName() const override
		{
			return "VosimOscillator";
		}

		double m_pulse_step;
		int m_curr_pulse_num;
		double m_curr_pulse_gain;
	};

	class FormantOscillator : public BasicOscillator
	{
	public:

		FormantOscillator(string name) :
			BasicOscillator(name),
			m_width(addDoubleParam("width", 0, 1, 0.5, 1e-3)),
			m_fmtpitch(addDoubleParam("fmtpitch", 0, 1, 0.5, 1e-3))
		{
			m_waveform.setCanEdit(false);
			m_waveform.setCanModulate(false);
		};

		FormantOscillator(const FormantOscillator& other) :
			FormantOscillator(other.m_name) {}

		void process(int bufind) override;
		UnitParameter& m_width;
		UnitParameter& m_fmtpitch;
	private:
		Unit* cloneImpl() const override
		{
			return new FormantOscillator(*this);
		}

		string getClassName() const override
		{
			return "FormantOscillator";
		}
	};
}

#endif // __VosimOscillator__


