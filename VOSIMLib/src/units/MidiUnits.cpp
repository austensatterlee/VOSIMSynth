#include "vosimlib/units/MidiUnits.h"
#include "vosimlib/DSPMath.h"

void syn::MidiNoteUnit::process_()
{
    BEGIN_PROC_FUNC
        WRITE_OUTPUT(oPitch, note());
        WRITE_OUTPUT(oFreq, pitchToFreq(note()));
        WRITE_OUTPUT(oPitchWheel, m_pitchWheelValue);
    END_PROC_FUNC
}

void syn::GateUnit::process_() {
    BEGIN_PROC_FUNC
        // Trigger sends a 1 and then turns off.
        if (isNoteOn()) {
            if (!m_triggerFired) {
                WRITE_OUTPUT(oTrig, 1.0);
                m_triggerFired = true;
            } else {
                WRITE_OUTPUT(oTrig, 0.0);
            }
        }
        // Gate sends a 0 first, then 1s until note off.
        if (m_queuedNoteOff) {
            WRITE_OUTPUT(oGate, 0.0);
            m_queuedNoteOff = false;
        } else {
            WRITE_OUTPUT(oGate, isNoteOn() ? 1.0 : 0.0);
        }
    END_PROC_FUNC
}

void syn::GateUnit::onNoteOff_() {
    m_queuedNoteOff = true;
    m_triggerFired = false;
}

void syn::MidiCCUnit::process_() {
    BEGIN_PROC_FUNC
        WRITE_OUTPUT(0, m_value);
    END_PROC_FUNC
}

void syn::MidiCCUnit::onParamChange_(int a_paramId) {
    if (a_paramId == m_pCC) {
        m_value = 0;
        setParam(m_pLearn, false);
    }
}

void syn::MidiCCUnit::onMidiControlChange_(int a_cc, double a_value) {
    if (param(m_pLearn).getBool()) {
        setParam(m_pCC, a_cc);
    }
    if (a_cc == param(m_pCC).getInt()) {
        m_value = a_value;
    }
}
