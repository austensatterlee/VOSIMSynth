// /** \file MathUnits.h
//      \brief 
//      \details
//      \author Austen Satterlee
//      \date 24/01/2016
// */

#include "Unit.h"

namespace syn
{
	class RectifierUnit : public Unit
	{
	public:
		RectifierUnit(const string& name) :
			Unit(name), m_input(addDoubleParam("input", -1, 1, 0.0, 1e-3, 1.0, false, true)),
			m_gain(addDoubleParam("gain", 0, 1, 1.0, 1e-3)) {}

		RectifierUnit(const RectifierUnit& other) :
			RectifierUnit(other.m_name) {}


	protected:
		void process(int bufind) override
		{
			m_output[bufind][0] = abs(m_input[0]) * m_gain;
			m_output[bufind][1] = abs(m_input[1]) * m_gain;
		};

		void onSampleRateChange(double newfs) override {};

	private:
		UnitParameter& m_input;
		UnitParameter& m_gain;

		Unit* cloneImpl() const override
		{
			return new RectifierUnit(*this);
		}

		string getClassName() const override
		{
			return "RectifierUnit";
		}
	};

	class InvertingUnit : public Unit
	{
	public:
		InvertingUnit(const string& name) :
			Unit(name),
			m_input(addDoubleParam("input", -1, 1, 0.0, 1e-3, 1.0, false, true)),
			m_gain(addDoubleParam("gain", 0, 1, 1.0, 1e-3)) {}

		InvertingUnit(const InvertingUnit& other) :
			InvertingUnit(other.m_name) {}


	protected:
		void process(int bufind) override
		{
			m_output[bufind][0] = -m_input[0] * m_gain;
			m_output[bufind][1] = -m_input[1] * m_gain;
		};

		void onSampleRateChange(double newfs) override {};

	private:
		UnitParameter& m_input;
		UnitParameter& m_gain;

		Unit* cloneImpl() const override
		{
			return new InvertingUnit(*this);
		}

		string getClassName() const override
		{
			return "InvertingUnit";
		}
	};

	class PanningUnit : public Unit
	{
	public:
		PanningUnit(const string& name)
			: Unit(name),
			  m_input(addDoubleParam("input", -1, 1, 0.0, 1e-3, 1.0, false, true)),
			  m_pan(addDoubleParam("pan", -1, 1, 0.0, 1e-3)),
			  m_gain(addDoubleParam("gain", 0, 1, 1.0, 1e-3)) {}

		PanningUnit(const PanningUnit& other) :
			PanningUnit(other.m_name) {}

	protected:

		void process(int bufind) override
		{
			double right_channel_amt = (1 + m_pan) * 0.5;
			m_output[bufind][0] = m_input[0] * (1 - right_channel_amt) * m_gain;
			m_output[bufind][1] = m_input[1] * right_channel_amt * m_gain;
		}

		void onSampleRateChange(double newfs) override {};

	private:
		UnitParameter& m_input;
		UnitParameter& m_pan;
		UnitParameter& m_gain;

		Unit* cloneImpl() const override
		{
			return new PanningUnit(*this);
		}

		string getClassName() const override
		{
			return "PanningUnit";
		}
	};

	class MemoryUnit : public Unit
	{
	public:
		MemoryUnit(const string& name) :
			Unit(name),
			m_state{ 0,0 },
			m_input(addDoubleParam("input", -1.0, 1.0, 0.0, 1e-3, 1.0, false, true)) {}

		MemoryUnit(const MemoryUnit& other) :
			MemoryUnit(other.m_name) {}

		void process(int bufind) override {
			m_output[bufind][0] = m_state[0];
			m_output[bufind][1] = m_state[1];
			m_state[0] = m_input[0];
			m_state[1] = m_input[1];
		}

		void onSampleRateChange(double newfs) override {};
	protected:
		double m_state[2];
	private:
		UnitParameter& m_input;
		Unit* cloneImpl() const override
		{
			return new MemoryUnit(*this);
		}

		string getClassName() const override
		{
			return "MemoryUnit";
		}
	};
}

