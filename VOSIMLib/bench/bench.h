#pragma once
#include "vosimlib/VoiceManager.h"
#include "vosimlib/units/StateVariableFilter.h"
#include "vosimlib/units/OscillatorUnit.h"
#include "vosimlib/units/MidiUnits.h"
#include "vosimlib/units/MathUnits.h"
#include <vosimlib/lut_tables.h>

inline syn::Circuit makeTestCircuit() {
    syn::lut_bl_tri_table();
    syn::lut_bl_saw_table();
    syn::lut_bl_square_table();

    const int nFiltUnits = 32;
    const int nOscUnits = 8;
    syn::Circuit mycircuit;
    // Add a bunch of filter units in serial
    syn::OnePoleLPUnit* lp[nFiltUnits];
    lp[0] = new syn::OnePoleLPUnit;
    mycircuit.addUnit(lp[0]);
    for (int i = 1; i < nFiltUnits; i++) {
        lp[i] = new syn::OnePoleLPUnit;
        mycircuit.addUnit(lp[i]);
        mycircuit.connectInternal(mycircuit.getUnitId(*lp[i - 1]), 0, mycircuit.getUnitId(*lp[i]), 0);
    }
    // Connect final ladder unit to circuit output
    mycircuit.connectInternal(mycircuit.getUnitId(*lp[nFiltUnits - 1]), 0, mycircuit.getOutputUnitId(), 0);
    // Add summation for accumulating oscillator outputs
    auto* sum = new syn::SummerUnit;
    mycircuit.addUnit(sum);
    // Connect summation to first filter
    mycircuit.connectInternal(mycircuit.getUnitId(*sum), 0, mycircuit.getUnitId(*lp[0]), 0);
    // Add pitch and oscillator units
    auto* mnu = new syn::MidiNoteUnit;
    mycircuit.addUnit(mnu);
    syn::BasicOscillatorUnit* bosc[nOscUnits];
    for (int i = 1; i < nOscUnits; i++) {
        bosc[i] = new syn::BasicOscillatorUnit;
        mycircuit.addUnit(bosc[i]);
        mycircuit.connectInternal(mycircuit.getUnitId(*mnu), 0, mycircuit.getUnitId(*bosc[i]), syn::TunedOscillatorUnit::Input::iNote) ;
        mycircuit.connectInternal(mycircuit.getUnitId(*bosc[i]), 0, mycircuit.getUnitId(*sum), i-1);
    }
    return mycircuit;
}
