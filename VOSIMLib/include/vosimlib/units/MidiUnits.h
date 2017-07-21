/**
 * \file units/MidiUnits.h
 * \brief
 * \details
 * \author Austen Satterlee
 * \date March 6, 2016
 */
#ifndef __MIDIUNITS__
#define __MIDIUNITS__
#include "vosimlib/Unit.h"
#include "vosimlib/Circuit.h"

namespace syn {
    /**
     * Outputs the current midi note
     */
    class VOSIMLIB_API MidiNoteUnit : public Unit {
        DERIVE_UNIT(MidiNoteUnit)
    public:
        explicit MidiNoteUnit(const string& a_name) :
            Unit(a_name) {
            addOutput_("pitch");
            addOutput_("freq");
        }

        MidiNoteUnit(const MidiNoteUnit& a_rhs) :
            MidiNoteUnit(a_rhs.name()) { }

        void reset() override {};

    protected:
        void process_() override;
    };

    /**
     * Outputs the current velocity
     */
    class VOSIMLIB_API VelocityUnit : public Unit {
        DERIVE_UNIT(VelocityUnit)
    public:
        explicit VelocityUnit(const string& a_name) :
            Unit(a_name) {
            addOutput_("out");
        }

        VelocityUnit(const VelocityUnit& a_rhs) :
            VelocityUnit(a_rhs.name()) { }

        void reset() override {};

    protected:
        void process_() override {
            BEGIN_PROC_FUNC
            WRITE_OUTPUT(0, velocity() * 0.0078125);
            END_PROC_FUNC
        };
    };

    /**
    * Outputs a 1 when a key is pressed, and 0 otherwise
    */
    class VOSIMLIB_API GateUnit : public Unit {
        DERIVE_UNIT(GateUnit)
    public:
        enum Output
        {
            oGate = 0,
            oTrig,
            NUM_OUTPUTS
        };

        explicit GateUnit(const string& a_name) :
            Unit(a_name),
            m_queuedNoteOff(true),
            m_triggerFired(false)
        {
            addOutput_(oGate, "gate");
            addOutput_(oTrig, "trig");
        }

        GateUnit(const GateUnit& a_rhs) :
            GateUnit(a_rhs.name()) { }

        void reset() override { m_queuedNoteOff = true; }

    protected:
        void process_() override {
            BEGIN_PROC_FUNC
            // Trigger sends a 1 and then turns off.
            if (isNoteOn()){
                if (!m_triggerFired) {
                    WRITE_OUTPUT(oTrig, 1.0);
                    m_triggerFired = true;
                }else {
                    WRITE_OUTPUT(oTrig, 0.0);
                }
            }
            // Gate sends a 0 first, then 1s until note off.
            if (m_queuedNoteOff) {
                WRITE_OUTPUT(oGate, 0.0);
                m_queuedNoteOff = false;
            }else{                
                WRITE_OUTPUT(oGate, isNoteOn() ? 1.0 : 0.0);
            }
            END_PROC_FUNC
        }

        void onNoteOff_() override { m_queuedNoteOff = true; m_triggerFired = false; }
    private:
        bool m_queuedNoteOff;
        bool m_triggerFired;
    };

    /**
     * Outputs the value of the selected Midi CC
     */
    class VOSIMLIB_API MidiCCUnit : public Unit {
        DERIVE_UNIT(MidiCCUnit)
    public:
        explicit MidiCCUnit(const string& a_name) :
            Unit(a_name),
            m_pCC(addParameter_(UnitParameter("CC", 0, 128, 0))),
            m_pLearn(addParameter_(UnitParameter("learn", false))),
            m_value(0) {
            addOutput_("out");
        }

        MidiCCUnit(const MidiCCUnit& a_rhs) :
            MidiCCUnit(a_rhs.name()) { }

        void reset() override {};

    protected:
        void process_() override {
            BEGIN_PROC_FUNC
            WRITE_OUTPUT(0, m_value); // divide by 128
            END_PROC_FUNC
        };

        void onParamChange_(int a_paramId) override {
            if (a_paramId == m_pCC) {
                m_value = 0;
                setParam(m_pLearn, false);
            }
        }

        void onMidiControlChange_(int a_cc, double a_value) override {
            if (param(m_pLearn).getBool()) {
                setParam(m_pCC, a_cc);
            }
            if (a_cc == param(m_pCC).getInt()) {
                m_value = a_value;
            }
        }

    private:
        int m_pCC, m_pLearn;
        double m_value;
    };


    /**
     * Outputs the index of the current voice.
     */
    class VOSIMLIB_API VoiceIndexUnit : public Unit {
        DERIVE_UNIT(VoiceIndexUnit)

    public:
        explicit VoiceIndexUnit(const string& a_name) :
            Unit(a_name) {
            addOutput_("out");
        }

        void reset() override {};
    protected:
        void process_() override {
            BEGIN_PROC_FUNC
            WRITE_OUTPUT(0, parent() ? parent()->getVoiceIndex() : 0.0);
            END_PROC_FUNC
        };
    };
}

#endif
