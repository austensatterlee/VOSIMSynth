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
 * \file MathUnits.h
 * \brief
 * \details
 * \author Austen Satterlee
 * \date 24/01/2016
 */

#ifndef __MATHUNITS__
#define __MATHUNITS__

#include "Unit.h"
#include <cmath>
#include <DSPMath.h>

using namespace std;

namespace syn {
	class MovingAverage
	{
	public:
		MovingAverage() : 
			m_windowSize(1),
			m_lastOutput(0.0)
		{
			m_delay.resizeBuffer(m_windowSize);
		}

		void setWindowSize(int a_newWindowSize) {
			m_windowSize = a_newWindowSize;
			m_delay.resizeBuffer(m_windowSize);
			m_delay.clearBuffer();
			m_lastOutput = 0.0;
		}

		double getWindowSize() const {
			return m_windowSize;
		}

		double process(double a_input) {
			double output = (1.0 / m_windowSize)*(a_input - m_delay.process(a_input)) + m_lastOutput;
			m_lastOutput = output;
			return output;
		}

		double getPastInputSample(int a_offset) {
			return m_delay.getPastSample(a_offset);
		}

	private:
		int m_windowSize;
		NSampleDelay m_delay;
		double m_lastOutput;
	};

	/**
	* DC-remover
	*/
	class DCRemoverUnit : public Unit {	
	public:
		DCRemoverUnit(const string& a_name) :
			Unit(a_name),
			m_pAlpha(addParameter_(UnitParameter("hp",0.0,1.0,0.95))),
			m_g(0.0)
		{
			addInput_("in");
			addOutput_("out");
		}

		DCRemoverUnit(const DCRemoverUnit& a_rhs) :
			DCRemoverUnit(a_rhs.getName())
		{

		}

	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double input = a_inputs.getValue(0);
			double alpha = getParameter(m_pAlpha).getDouble();
			// dc removal
			double last_g = m_g;
			m_g = input + alpha*last_g;
			double output = m_g - last_g;
			a_outputs.setChannel(0, output);
		}; 

		void onParamChange_(int a_paramId) override {
			if (a_paramId == m_pAlpha) {
			}
		}
	private:
		string _getClassName() const override
		{
			return "DCRemoverUnit";
		}

		Unit* _clone() const override { return new DCRemoverUnit(*this); }

	private:
		int m_pAlpha;
		double m_g;
	};

	/**
	 * Full-wave rectifier
	 */
    class FullRectifierUnit : public Unit {
    public:
		FullRectifierUnit(const string& a_name) :
                Unit(a_name)
        {
            addInput_("in");
            addOutput_("out");
        }

		FullRectifierUnit(const FullRectifierUnit& a_rhs) :
			FullRectifierUnit(a_rhs.getName())
        {

        }

    protected:
        void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
        {
            double input = a_inputs.getValue(0);
            a_outputs.setChannel(0, abs(input));
        };
    private:
        string _getClassName() const override
        {
            return "FullRectifierUnit";
        }
        virtual Unit* _clone() const { return new FullRectifierUnit(*this); }
    };

	/**
	* Half-wave rectifier
	*/
	class HalfRectifierUnit : public Unit {
	public:
		HalfRectifierUnit(const string& a_name) :
			Unit(a_name)
		{
			addInput_("in");
			addOutput_("out");
		}

		HalfRectifierUnit(const HalfRectifierUnit& a_rhs) :
			HalfRectifierUnit(a_rhs.getName())
		{

		}

	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double input = a_inputs.getValue(0);
			input = input > 0 ? input : 0;
			a_outputs.setChannel(0, input);
		};
	private:
		string _getClassName() const override
		{
			return "HalfRectifierUnit";
		}
		virtual Unit* _clone() const { return new HalfRectifierUnit(*this); }
	};

	/**
	 * Flips the polarity of a signal
	 */
    class InvertingUnit : public Unit {
    public:
        InvertingUnit(const string& a_name) :
                Unit(a_name)
        {
            addInput_("in");
            addOutput_("out");
        }

        InvertingUnit(const InvertingUnit& a_rhs) :
                InvertingUnit(a_rhs.getName())
        {

        }

    protected:
        void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
        {
            double input = a_inputs.getValue(0);
            a_outputs.setChannel(0, -input);
        };
    private:
        string _getClassName() const override
        {
            return "InvertingUnit";
        }

	    Unit* _clone() const override { return new InvertingUnit(*this); }
    };

	/**
	 * Multiplies two signals together
	 */
	class MultiplyUnit : public Unit {
	public:
		MultiplyUnit(const string& a_name) :
			Unit(a_name)
		{
			addInput_("in1");
			addInput_("in2[x]", 1.0, Signal::EMul);
			addOutput_("out");
		}

		MultiplyUnit(const MultiplyUnit& a_rhs) :
			MultiplyUnit(a_rhs.getName())
		{

		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double output = 0.0;
			output += a_inputs.getValue(0);
			output *= a_inputs.getValue(1);
			a_outputs.setChannel(0, output);
		};
	private:
		string _getClassName() const override
		{
			return "MultiplyUnit";
		}

		Unit* _clone() const override { return new MultiplyUnit(*this); }
	};

	/**
	* Multiplies a signal by a constant
	*/
	class GainUnit : public Unit {
	public:
		GainUnit(const string& a_name) :
			Unit(a_name),
			m_pGain(addParameter_(UnitParameter("gain", 0.0, 10.0, 1.0)))
		{
			addInput_("in");
			addOutput_("out");
		}

		GainUnit(const MultiplyUnit& a_rhs) :
			GainUnit(a_rhs.getName())
		{

		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double output = a_inputs.getValue(0);
			output *= getParameter(m_pGain).getDouble();
			a_outputs.setChannel(0, output);
		};
	private:
		string _getClassName() const override
		{
			return "GainUnit";
		}

		Unit* _clone() const override { return new GainUnit(*this); }
	private:
		int m_pGain;
	};

	/**
	* Sums incomming signals
	*/
	class SummerUnit : public Unit {
	public:
		SummerUnit(const string& a_name) :
			Unit(a_name)
		{
			addInput_("in");
			addOutput_("out");
		}

		SummerUnit(const SummerUnit& a_rhs) :
			SummerUnit(a_rhs.getName())
		{

		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double output = a_inputs.getValue(0);
			a_outputs.setChannel(0, output);
		};
	private:
		string _getClassName() const override
		{
			return "SummerUnit";
		}

		Unit* _clone() const override { return new SummerUnit(*this); }
	};

	/**
	 * Outputs a constant
	 */
	const vector<string> scale_selections = { "1","10","100" };
	class ConstantUnit : public Unit {
	public:
		ConstantUnit(const string& a_name) :
			Unit(a_name)
		{
			addParameter_({ "out",-1.0,1.0,1.0 });
			addParameter_({ "scale",scale_selections});
			addOutput_("out");
		}

		ConstantUnit(const ConstantUnit& a_rhs) :
			ConstantUnit(a_rhs.getName())
		{
		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double output = getParameter(0).getDouble();
			int selected_scale = getParameter(1).getInt();
			double scale = 1.0;
			switch(selected_scale) {
			case 0:
				scale = 1.0;
				break;
			case 1:
				scale = 10.0;
				break;
			case 2:
				scale = 100.0;
				break;
			default:
				scale = 1.0;
				break;
			}
			output = output*scale;
			a_outputs.setChannel(0, output);
		};
	private:
		string _getClassName() const override
		{
			return "ConstantUnit";
		}

		Unit* _clone() const override { return new ConstantUnit(*this); }
	};

	/**
	* Balances incoming signals between two outputs
	*/
	class PanningUnit : public Unit {
	public:
		PanningUnit(const string& a_name) :
			Unit(a_name)
		{
			addInput_("in1");
			addInput_("in2");
			addInput_("bal");
			addOutput_("out1");
			addOutput_("out2");
			m_pBalance = addParameter_({ "balance",-1.0,1.0,0.0 });
		}

		PanningUnit(const PanningUnit& a_rhs) :
			PanningUnit(a_rhs.getName())
		{

		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double in1 = a_inputs.getValue(0);
			double in2 = a_inputs.getValue(1);
			double balance = 0.5*(getParameter(m_pBalance).getDouble()+a_inputs.getValue(2)+1);
			balance = CLAMP(balance, 0.0, 1.0);
			a_outputs.setChannel(0, in1*(balance)+in2*(1 - balance));
			a_outputs.setChannel(1, in2*(balance)+in1*(1 - balance));
		};
	protected:
		int m_pBalance;
	private:
		string _getClassName() const override
		{
			return "PanningUnit";
		}

		Unit* _clone() const override { return new PanningUnit(*this); }
	};
}

#endif