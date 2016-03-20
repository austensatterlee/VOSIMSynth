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
#include "DSPMath.h"
#include <cmath>
#include "MemoryUnit.h"

using namespace std;

namespace syn {

	const vector<string> scale_selections = { "1","10","100" };
	const vector<double> scale_values = { 1.,10.,100. };

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
	 * Bilinear/trapezoidal integrator
	 */
	class BLIntegrator
	{
	public:
		BLIntegrator() : m_state(0), m_normfc(0) {};
		/**
		 * Set normalized cutoff frequency in the range (0,1), where 1 is nyquist.
		 */
		void setFc(double a_newfc) { m_normfc = a_newfc/2; }		
		double process(double a_input) {
			a_input = m_normfc*a_input;
			m_state = 2 * a_input + m_state;
			return m_state;
		}
	private:
		double m_state;
		double m_normfc; /// normalized cutoff frequency
	};

	/**
	 * DC-remover
	 */
	class DCRemoverUnit : public Unit {	
	public:
		DCRemoverUnit(const string& a_name) :
			Unit(a_name),
			m_pAlpha(addParameter_(UnitParameter("hp",0.0,1.0,0.995))),
			m_lastInput(0.0),
			m_lastOutput(0.0)
		{
			addInput_("in");
			addOutput_("out");
		}

		DCRemoverUnit(const DCRemoverUnit& a_rhs) :
			DCRemoverUnit(a_rhs.getName())
		{}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double input = a_inputs.getValue(0);
			double alpha = getParameter(m_pAlpha).getDouble();
			double gain = 0.5*(1 + alpha);
			// dc removal			
			input = input*gain;
			double output = input - m_lastInput + alpha*m_lastOutput;
			m_lastInput = input;
			m_lastOutput = output;
			a_outputs.setChannel(0, output);
		}; 

	private:
		string _getClassName() const override
		{
			return "DCRemoverUnit";
		}

		Unit* _clone() const override { return new DCRemoverUnit(*this); }

	private:
		int m_pAlpha;
		double m_lastOutput;
		double m_lastInput;
	};


	/**
	* 1 Pole Filter (Lag)
	*/
	class LagUnit : public Unit
	{
	public:
		LagUnit(const string& a_name) : 
			Unit(a_name),
			m_pFc(addParameter_(UnitParameter("fc",0.0,1.0,1.0))),
			m_state(0.0) 
		{
			addInput_("in");
			m_iFcAdd = addInput_("fc");
			m_iFcMul = addInput_("fc[x]",1.0,Signal::EMul);
			addOutput_("out");
		}

		LagUnit(const LagUnit& a_rhs) : LagUnit(a_rhs.getName()) {}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override {
			double input = a_inputs.getValue(0);
			double fc = getParameter(m_pFc).getDouble()*a_inputs.getValue(m_iFcMul)+a_inputs.getValue(m_iFcAdd); // pitch cutoff
			fc = 10e3*CLAMP(fc, 0.0, 1.0) / getFs();
			double wc = 2*tan(PI * fc / 2.0);
			double gain = wc / (1 + wc);
			double trap_in = gain * (input - m_state);
			double output = trap_in + m_state;
			m_state = trap_in + output;
			a_outputs.setChannel(0, output);
		}
	private:
		string _getClassName() const override { return "LagUnit";  };
		Unit* _clone() const override { return new LagUnit(*this);  };
	private:
		int m_pFc;
		int m_iFcAdd, m_iFcMul;
		double m_state;
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

	    Unit* _clone() const override { return new FullRectifierUnit(*this); }
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

		Unit* _clone() const override { return new HalfRectifierUnit(*this); }
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
	class MACUnit : public Unit {
	public:
		MACUnit(const string& a_name) :
			Unit(a_name)
		{
			addInput_("in");
			addInput_("a[x]", 1.0, Signal::EMul);
			addInput_("b[+]");
			addOutput_("a*in+b");
		}

		MACUnit(const MACUnit& a_rhs) :
			MACUnit(a_rhs.getName())
		{

		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double output = a_inputs.getValue(0);
			output *= a_inputs.getValue(1);
			output += a_inputs.getValue(2);
			a_outputs.setChannel(0, output);
		};
	private:
		string _getClassName() const override
		{
			return "MACUnit";
		}

		Unit* _clone() const override { return new MACUnit(*this); }
	};

	/**
	 * Applies gain to the difference between the two inputs (like an op amp)
	 */
	class GainUnit : public Unit {
	public:
		GainUnit(const string& a_name) :
			Unit(a_name),
			m_pGain(addParameter_(UnitParameter("gain", 0.0, 1.0, 1.0))),
			m_pScale(addParameter_(UnitParameter("scale",scale_selections)))			
		{
			m_iInput = addInput_("in[+]");
			m_iInvInput = addInput_("in[-]");
			m_iGain = addInput_("gain[x]", 1.0, Signal::EMul);
			addOutput_("out");
		}

		GainUnit(const GainUnit& a_rhs) :
			GainUnit(a_rhs.getName())
		{

		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double input = a_inputs.getValue(m_iInput) - a_inputs.getValue(m_iInvInput);
			double gain = getParameter(m_pGain).getDouble()*scale_values[getParameter(m_pScale).getInt()];
			gain *= a_inputs.getValue(m_iGain);
			a_outputs.setChannel(0, input*gain);
		};
	private:
		string _getClassName() const override
		{
			return "GainUnit";
		}

		Unit* _clone() const override { return new GainUnit(*this); }
	private:
		int m_pGain, m_pScale;
		int m_iGain, m_iInput, m_iInvInput;
	};

	/**
	* Sums incomming signals
	*/
	class SummerUnit : public Unit {
	public:
		SummerUnit(const string& a_name) :
			Unit(a_name),
			m_pBias(addParameter_(UnitParameter("bias", -1.0, 1.0, 0.0))),
			m_pScale(addParameter_(UnitParameter("bias scale", scale_selections)))
		{
			addInput_("[+]");
			addInput_("[-]");
			addOutput_("out");
		}

		SummerUnit(const SummerUnit& a_rhs) :
			SummerUnit(a_rhs.getName())
		{

		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double output = a_inputs.getValue(0) - a_inputs.getValue(1);
			output += getParameter(m_pBias).getDouble()*scale_values[getParameter(m_pScale).getInt()];
			a_outputs.setChannel(0, output);
		};
	private:
		string _getClassName() const override
		{
			return "SummerUnit";
		}

		Unit* _clone() const override { return new SummerUnit(*this); }
	private:
		int m_pBias, m_pScale;
	};

	/**
	 * Outputs a constant
	 */
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
			double scale = scale_values[selected_scale];
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
			m_pBalance = addParameter_({ "balance",0.0,1.0,0.5 });
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
			double balance = getParameter(m_pBalance).getDouble()+a_inputs.getValue(2);
			balance = CLAMP(balance, 0.0, 1.0);
			a_outputs.setChannel(0,     balance*in1 + (1-balance)*in2);
			a_outputs.setChannel(1, (1-balance)*in1 +     balance*in2);
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

	/**
	 * Linear to decibals converter
	 */
	class LinToDbUnit : public Unit {
	public:
		LinToDbUnit(const string& a_name) :
			Unit(a_name),
			m_pMinDb(addParameter_(UnitParameter("min dB", -120.0, 0.0, -120.0)))
		{
			addInput_("in");
			addOutput_("out");
		}

		LinToDbUnit(const SummerUnit& a_rhs) :
			LinToDbUnit(a_rhs.getName())
		{
		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double input = a_inputs.getValue(0);
			double output = lin2db(input, getParameter(m_pMinDb).getDouble(), 0.0);
			a_outputs.setChannel(0, output);
		};
	private:
		string _getClassName() const override
		{
			return "LinToDbUnit";
		}

		Unit* _clone() const override { return new LinToDbUnit(*this); }
	private:
		int m_pMinDb;
	};

	/**
	* Affine transform
	*/
	class LerpUnit : public Unit {
	public:
		LerpUnit(const string& a_name) :
			Unit(a_name)
		{
			m_pInputRange = addParameter_(UnitParameter("input", { "bipolar","unipolar" }));
			m_pMinOutput = addParameter_(UnitParameter("min out", -1.0, 1.0, 0.0));
			m_pMinOutputScale = addParameter_(UnitParameter("min scale", scale_selections));
			m_pMaxOutput = addParameter_(UnitParameter("max out", -1.0, 1.0, 1.0));
			m_pMaxOutputScale = addParameter_(UnitParameter("max scale", scale_selections));
			addInput_("in");
			addOutput_("out");
		}

		LerpUnit(const SummerUnit& a_rhs) :
			LerpUnit(a_rhs.getName())
		{
		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double input = a_inputs.getValue(0);
			double a_scale = scale_values[getParameter(m_pMinOutputScale).getInt()];
			double a = getParameter(m_pMinOutput).getDouble()*a_scale;
			double b_scale = scale_values[getParameter(m_pMaxOutputScale).getInt()];
			double b = getParameter(m_pMaxOutput).getDouble()*b_scale;
			double output;
			if (getParameter(m_pInputRange).getInt() == 1) {
				output = LERP(a, b, input);
			}else {
				output = LERP(a, b, 0.5*(input + 1));
			}
			a_outputs.setChannel(0, output);
		};
	private:
		string _getClassName() const override
		{
			return "LerpUnit";
		}

		Unit* _clone() const override { return new LerpUnit(*this); }
	private:
		int m_pInputRange;
		int m_pMinOutput, m_pMaxOutput;
		int m_pMinOutputScale, m_pMaxOutputScale;
	};
}

#endif