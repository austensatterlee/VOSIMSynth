/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"

#include "Oscillator.h"
#include "VosimOscillator.h"
#include "ADSREnvelope.h"
#include "Follower.h"
#include "MathUnits.h"
#include "MemoryUnit.h"
#include "MidiUnits.h"
#include "StateVariableFilter.h"
#include "WaveShapers.h" 
#include "MainWindow.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"
#include "UnitFactory.h"

#include <tables.h>
#include "MainGUI.h"

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
	:
	IPLUG_CTOR(0, 1, instanceInfo), m_tempo(0) {
	TRACE;
	makeInstrument();
	makeGraphics();
	syn::lut_bl_tri_table();
	syn::lut_bl_saw_table();
	syn::lut_bl_square_table();
}

void VOSIMSynth::makeGraphics() {
	syn::VoiceManager* vm = m_voiceManager;
	syn::UnitFactory* uf = m_unitFactory;
	synui::MainWindow* vosimWindow = new synui::MainWindow(GUI_WIDTH, GUI_HEIGHT, [vm,uf](synui::MainWindow& a_win){ return new synui::MainGUI(a_win, vm, uf); });
	vosimWindow->setHInstance(gHInstance);
	AttachAppWindow(vosimWindow);
}

void VOSIMSynth::makeInstrument() {
	m_unitFactory = &syn::UnitFactory::instance();
	m_unitFactory->addUnitPrototype<syn::StateVariableFilter>("Filters", "svf");
	m_unitFactory->addUnitPrototype<syn::TrapStateVariableFilter>("Filters", "tsvf");
	m_unitFactory->addUnitPrototype<syn::OnePoleLP>("Filters", "lag");
	m_unitFactory->addUnitPrototype<syn::LadderFilter>("Filters", "ladderA");
	m_unitFactory->addUnitPrototype<syn::LadderFilterTwo>("Filters", "ladderB");

	m_unitFactory->addUnitPrototype<syn::BasicOscillator>("Oscillators", "basic");
	m_unitFactory->addUnitPrototype<syn::VosimOscillator>("Oscillators", "vosim");
	m_unitFactory->addUnitPrototype<syn::FormantOscillator>("Oscillators", "formant");

	m_unitFactory->addUnitPrototype<syn::ADSREnvelope>("Modulators", "ADSR");
	m_unitFactory->addUnitPrototype<syn::LFOOscillator>("Modulators", "LFO");

	m_unitFactory->addUnitPrototype<syn::MemoryUnit>("DSP", "unit delay");
	m_unitFactory->addUnitPrototype<syn::VariableMemoryUnit>("DSP", "var delay");
	m_unitFactory->addUnitPrototype<syn::PanningUnit>("DSP", "pan");
	m_unitFactory->addUnitPrototype<syn::FollowerUnit>("DSP", "follow");
	m_unitFactory->addUnitPrototype<syn::DCRemoverUnit>("DSP", "dc trap");

	m_unitFactory->addUnitPrototype<syn::ConstantUnit>("Math", "const");
	m_unitFactory->addUnitPrototype<syn::SummerUnit>("Math", "sum");
	m_unitFactory->addUnitPrototype<syn::GainUnit>("Math", "gain");
	m_unitFactory->addUnitPrototype<syn::LerpUnit>("Math", "affine");
	m_unitFactory->addUnitPrototype<syn::RectifierUnit>("Math", "rect");
	m_unitFactory->addUnitPrototype<syn::PitchToFreqUnit>("Math", "p2f");
	m_unitFactory->addUnitPrototype<syn::FreqToPitchUnit>("Math", "f2p");
	m_unitFactory->addUnitPrototype<syn::TanhUnit>("Math", "tanh");
	m_unitFactory->addUnitPrototype<syn::SwitchUnit>("Math", "switch");

	m_unitFactory->addUnitPrototype<syn::GateUnit>("MIDI", "gate");
	m_unitFactory->addUnitPrototype<syn::MidiNoteUnit>("MIDI", "pitch");
	m_unitFactory->addUnitPrototype<syn::VelocityUnit>("MIDI", "vel");
	m_unitFactory->addUnitPrototype<syn::MidiCCUnit>("MIDI", "CC");

	m_unitFactory->addUnitPrototype<syn::InputUnit>("", "in");
	m_unitFactory->addUnitPrototype<syn::OutputUnit>("", "out");

	m_voiceManager = new syn::VoiceManager(m_unitFactory);
	m_voiceManager->setMaxVoices(8);

	m_MIDIReceiver = new syn::MIDIReceiver(m_voiceManager);
}

VOSIMSynth::~VOSIMSynth() {
	delete m_voiceManager;
	delete m_MIDIReceiver;
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
	// Mutex is already locked for us.

	for (int s = 0; s < nFrames; s++) {
		m_MIDIReceiver->advance();
	}

	// If tempo has changed, notify instrument
	if (m_tempo != GetTempo()) {
		m_voiceManager->setTempo(GetTempo());
	}

	// Process samples
	m_voiceManager->tick(inputs[0], inputs[1], outputs[0], outputs[1]);

	m_MIDIReceiver->Flush(nFrames);
	m_tickCount++;
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg) {
	m_MIDIReceiver->onMessageReceived(pMsg);
}

bool VOSIMSynth::SerializeState(ByteChunk* pChunk) {
	std::shared_ptr<syn::Unit> circuit(m_voiceManager->getPrototypeCircuit()->clone());
	std::stringstream ss; {
		cereal::XMLOutputArchive ar(ss);
		ar(cereal::make_nvp("circuit", circuit));
	}
	pChunk->PutStr(ss.str().c_str());

	// <store gui circuit here>
	return true;
}

int VOSIMSynth::UnserializeState(ByteChunk* pChunk, int startPos) {
	m_unitFactory->resetBuildCounts();

	string input;
	startPos = pChunk->Get(&input, startPos);
	std::stringstream ss{ input };
	std::shared_ptr<syn::Unit> circuit;
	{
		cereal::XMLInputArchive ar(ss);
		ar(cereal::make_nvp("circuit", circuit));
	}
	// <reset gui circuit here>
	m_voiceManager->setPrototypeCircuit(*static_cast<const syn::Circuit*>(circuit.get()));
	Reset();
	// <load new gui circuit here>
	return startPos;
}

void VOSIMSynth::PresetsChangedByHost() {}

void VOSIMSynth::OnIdle() {
	m_voiceManager->onIdle();
}

void VOSIMSynth::OnActivate(bool active) {}

void VOSIMSynth::OnGUIOpen() {}

void VOSIMSynth::OnGUIClose() {}

void VOSIMSynth::Reset() {
	m_MIDIReceiver->Resize(GetBlockSize());
	m_voiceManager->setBufferSize(GetBlockSize());
	m_voiceManager->setFs(GetSampleRate());
}

void VOSIMSynth::OnParamChange(int paramIdx) {}