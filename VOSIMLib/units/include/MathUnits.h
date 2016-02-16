/**
 * \file MathUnits.h
 * \brief
 * \details
 * \author Austen Satterlee
 * \date 24/01/2016
 */

#include "Unit.h"
#include <cmath>

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
		virtual Unit* _clone() const { return new MultiplyUnit(*this); }
	};

	/**
	 * Outputs a constant
	 */
	class ConstantUnit : public Unit {
	public:
		ConstantUnit(const string& a_name) :
			Unit(a_name)
		{
			addParameter_({ "out",0.0,10.0,1.0 });
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
		virtual Unit* _clone() const { return new ConstantUnit(*this); }
	};

	/**
	* Outputs a constant
	*/
	class MemoryUnit : public Unit {
	public:
		MemoryUnit(const string& a_name) :
			Unit(a_name),
			m_lastInput(0)
		{
			addInput_("in");
			addOutput_("out");
		}

		MemoryUnit(const MemoryUnit& a_rhs) :
			MemoryUnit(a_rhs.getName())
		{

		}
	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override
		{
			double output = m_lastInput;
			m_lastInput = a_inputs.getValue(0);
			a_outputs.setChannel(0, output);
		};
	private:
		string _getClassName() const override
		{
			return "MemoryUnit";
		}

		Unit* _clone() const override { return new MemoryUnit(*this); }
	private:
		double m_lastInput;
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

		bool isActive() const override {
			return isNoteOn();
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

		bool isActive() const override {
			return isNoteOn();
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
}

