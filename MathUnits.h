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
			Unit(name), m_input(addDoubleParam("input", -1, 1, 0.0, 1e-3, 1.0, true, true)),
			m_gain(addDoubleParam("gain", -1, 1, 0.5, 1e-3)) {}

		RectifierUnit(const RectifierUnit& other) :
			RectifierUnit(other.m_name) {}


	protected:
		void process(int bufind) override
		{
			m_output[bufind] = abs(m_input) * m_gain;
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

	class AccumulatingUnit : public Unit
	{
	public:
		AccumulatingUnit(const string& name)
			: Unit(name),
			  m_input(addDoubleParam("input", -1, 1, 0.0, 1e-3, 1.0, true, true)),
			  m_gain(addDoubleParam("gain", 0, 1, 0.5, 1e-3)) {}

		AccumulatingUnit(const AccumulatingUnit& other) :
			AccumulatingUnit(other.m_name) {}
	protected:

		void process(int bufind) override
		{
			m_output[bufind] = m_input * m_gain;
		}

		void onSampleRateChange(double newfs) override {};

	private:
		UnitParameter& m_input;
		UnitParameter& m_gain;

		Unit* cloneImpl() const override
		{
			return new AccumulatingUnit(*this);
		}

		string getClassName() const override
		{
			return "AccumulatingUnit";
		}
	};
}

