#include "MidiUnits.h"
#include "DSPMath.h"

#include "common.h"





void syn::MidiNoteUnit::process_() {
    setOutputChannel_(0, note());
    setOutputChannel_(1, pitchToFreq(note()));
}
