/**
 * \file MathUnits.h
 * \brief
 * \details
 * \author Austen Satterlee
 * \date 24/01/2016
 */

#include "Unit.h"
#include <cmath>
#include <DSPMath.h>

using namespace std;

namespace syn {
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
        virtual Unit* _clone() const { return new InvertingUnit(*this); }
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
			addInput_("in2");
			addOutput_("out");
		}

		MultiplyUnit(const MultiplyUnit& a_rhs) :
			MultiplyUnit(a_rhs.getName())
		{

		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double output = 1.0;
			output *= a_inputs.getValue(0);
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
	* Outputs a constant
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
	class ConstantUnit : public Unit {
	public:
		ConstantUnit(const string& a_name) :
			Unit(a_name)
		{
			addParameter_({ "out",-100.0,100.0,1.0 });
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
	* Outputs a constant
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

	/**
	* N-Sample delay
	*/
	class MemoryUnit : public Unit {
	
	public:
		MemoryUnit(const string& a_name) :
			Unit(a_name),
			m_buffer(1,0),
			m_pBufSize(addParameter_(UnitParameter("samples",1,128,1))),
			m_bufferIndex(0)
		{
			addInput_("in");
			addOutput_("out");
		}

		MemoryUnit(const MemoryUnit& a_rhs) :
			MemoryUnit(a_rhs.getName())
		{

		}
	protected:
		void onParamChange_(int a_paramId) override {
			if(a_paramId == m_pBufSize) {
				m_buffer.resize(getParameter(m_pBufSize).getInt());
				m_bufferIndex = 0;
			}
		}
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			if (isnan(m_buffer[m_bufferIndex]) || isinf(m_buffer[m_bufferIndex])) {
				m_buffer[m_bufferIndex] = 0.0;
			}
			double output = m_buffer[m_bufferIndex];

			int bufferSize = getParameter(m_pBufSize).getInt();
			int bufferWriteIndex = WRAP<int>(m_bufferIndex - bufferSize, bufferSize);
			m_buffer[bufferWriteIndex] = a_inputs.getValue(0);

			m_bufferIndex++;
			if (m_bufferIndex >= bufferSize)
				m_bufferIndex = 0;

			a_outputs.setChannel(0, output);
		};
	private:
		string _getClassName() const override
		{
			return "MemoryUnit";
		}

		Unit* _clone() const override { return new MemoryUnit(*this); }
	private:
		vector<double> m_buffer;
		int m_bufferIndex;
		int m_pBufSize;
	};

	//-----------------------------------------------------------------------------------------
	// MIDI Units
	//-----------------------------------------------------------------------------------------

	/** 
	 * Outputs the current midi note
	 */
    class MidiNoteUnit : public Unit {
    public:
        MidiNoteUnit(const string& a_name) :
                Unit(a_name)
        {
            addOutput_("out");
        }

        MidiNoteUnit(const MidiNoteUnit& a_rhs) :
                MidiNoteUnit(a_rhs.getName())
        {

        }

    protected:
        void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
        {
            a_outputs.setChannel(0, getNote());
        };
    private:
        string _getClassName() const override
        {
            return "MidiNoteUnit";
        }

	    Unit* _clone() const override { return new MidiNoteUnit(*this); }
    };

	/**
	* Outputs the current midi note in the range (0.0,1.0)
	*/
	class NormalizedMidiNoteUnit : public Unit {
	public:
		NormalizedMidiNoteUnit(const string& a_name) :
			Unit(a_name)
		{
			addOutput_("out");
		}

		NormalizedMidiNoteUnit(const MidiNoteUnit& a_rhs) :
			NormalizedMidiNoteUnit(a_rhs.getName())
		{

		}

	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			a_outputs.setChannel(0, getNote()/128.0);
		};
	private:
		string _getClassName() const override
		{
			return "NormalizedMidiNoteUnit";
		}

		Unit* _clone() const override { return new NormalizedMidiNoteUnit(*this); }
	};

	/**
	 * Outputs the current velocity
	 */
	class VelocityUnit : public Unit {
	public:
		VelocityUnit(const string& a_name) :
			Unit(a_name)
		{
			addOutput_("out");
		}

		VelocityUnit(const MidiNoteUnit& a_rhs) :
			VelocityUnit(a_rhs.getName())
		{

		}

	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			a_outputs.setChannel(0, getVelocity()*1./128);
		};
	private:
		string _getClassName() const override
		{
			return "VelocityUnit";
		}

		Unit* _clone() const override { return new VelocityUnit(*this); }
	};

	/**
	* Outputs a 1 when a key is pressed, and 0 otherwise
	*/
	class GateUnit : public Unit {
	public:
		GateUnit(const string& a_name) :
			Unit(a_name)
		{
			addOutput_("out");
		}

		GateUnit(const MidiNoteUnit& a_rhs) :
			GateUnit(a_rhs.getName())
		{

		}

		bool isActive() const override {
			return isNoteOn();
		}

	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			a_outputs.setChannel(0, static_cast<double>(isNoteOn()));
		};
	private:
		string _getClassName() const override
		{
			return "GateUnit";
		}

		Unit* _clone() const override { return new GateUnit(*this); }
	};
}

