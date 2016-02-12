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
    class RectifierUnit : public Unit {
    public:
        RectifierUnit(const string& a_name) :
                Unit(a_name)
        {
            addInput_("in");
            addOutput_("out");
        }

        RectifierUnit(const RectifierUnit& a_rhs) :
                RectifierUnit(a_rhs.getName())
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
            return "RectifierUnit";
        }
        virtual Unit* _clone() const { return new RectifierUnit(*this); }
    };

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
        virtual Unit* _clone() const { return new MidiNoteUnit(*this); }
    };
}

