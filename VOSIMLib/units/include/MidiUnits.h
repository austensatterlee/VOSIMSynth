/**
 * \file MidiUnits.h
 * \brief
 * \details
 * \author Austen Satterlee
 * \date March 6, 2016
 */
#ifndef __MIDIUNITS__
#define __MIDIUNITS__

#include "Unit.h"

using namespace std;

namespace syn {
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
#endif
