#include "vosimlib/units/MidiUnits.h"
#include "vosimlib/DSPMath.h"

void syn::MidiNoteUnit::process_()
{
    BEGIN_PROC_FUNC
        WRITE_OUTPUT(0, note());
        WRITE_OUTPUT(1, pitchToFreq(note()));
    END_PROC_FUNC
}
