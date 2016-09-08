#include "MidiUnits.h"
#include "DSPMath.h"

#include "common.h"
CEREAL_REGISTER_TYPE(syn::MidiNoteUnit);
CEREAL_REGISTER_TYPE(syn::VelocityUnit);
CEREAL_REGISTER_TYPE(syn::GateUnit);
CEREAL_REGISTER_TYPE(syn::MidiCCUnit);

void syn::MidiNoteUnit::process_() {
	setOutputChannel_(0, note());
	setOutputChannel_(1, pitchToFreq(note()));
}
