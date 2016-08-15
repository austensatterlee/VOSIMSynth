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
#include "include/OscilloscopeUnit.h"
#include "include/SpectroscopeUnit.h"
#include "WaveShapers.h" 
#include "MainWindow.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"
#include "UnitFactory.h"
#include "include/UICircuitPanel.h"
#include "include/VOSIMComponent.h"
#include <cereal/archives/json.hpp>
#include <tables.h>

using namespace std;

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
	:
	IPLUG_CTOR(0, 1, instanceInfo), m_tempo(0) {
	TRACE;
	SYN_ENABLE_DENORMAL_ROUNDING();
	makeInstrument();
	makeGraphics();
	syn::lut_bl_tri_table();
	syn::lut_bl_saw_table();
	syn::lut_bl_square_table();
}

void VOSIMSynth::makeGraphics() {
	syn::VoiceManager* vm = m_voiceManager;
	syn::UnitFactory* uf = m_unitFactory;
	synui::MainWindow* vosimWindow = new synui::MainWindow(GUI_WIDTH, GUI_HEIGHT, [vm, uf](synui::MainWindow* a_win)-> synui::UIComponent* {
		return new synui::VOSIMComponent(a_win, vm, uf);
	});
	vosimWindow->setHInstance(gHInstance);
	AttachAppWindow(vosimWindow);

	getVOSIMComponent()->circuitPanel()->registerUnitControl<synui::OscilloscopeUnit>([](synui::MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId)-> synui::UIUnitControl* {
		return new synui::OscilloscopeUnitControl(a_window, a_vm, a_unitId);
	});
	getVOSIMComponent()->circuitPanel()->registerUnitControl<synui::SpectroscopeUnit>([](synui::MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId)-> synui::UIUnitControl* {
		return new synui::SpectroscopeUnitControl(a_window, a_vm, a_unitId);
	});
}

void VOSIMSynth::makeInstrument() {
	m_unitFactory = new syn::UnitFactory();
	m_unitFactory->addUnitPrototype<syn::StateVariableFilter>("Filters", "SVF");
	m_unitFactory->addUnitPrototype<syn::TrapStateVariableFilter>("Filters", "TSVF");
	m_unitFactory->addUnitPrototype<syn::OnePoleLP>("Filters", "Lag");
	m_unitFactory->addUnitPrototype<syn::LadderFilter>("Filters", "Ladder");

	m_unitFactory->addUnitPrototype<syn::BasicOscillator>("Oscillators", "Basic");
	m_unitFactory->addUnitPrototype<syn::VosimOscillator>("Oscillators", "VOSIM");
	m_unitFactory->addUnitPrototype<syn::FormantOscillator>("Oscillators", "Formant");

	m_unitFactory->addUnitPrototype<syn::ADSREnvelope>("Modulators", "ADSR");
	m_unitFactory->addUnitPrototype<syn::LFOOscillator>("Modulators", "LFO");

	m_unitFactory->addUnitPrototype<syn::MemoryUnit>("DSP", "Unit Delay");
	m_unitFactory->addUnitPrototype<syn::ResampleUnit>("DSP", "Var Delay");
	m_unitFactory->addUnitPrototype<syn::PanningUnit>("DSP", "Pan");
	m_unitFactory->addUnitPrototype<syn::FollowerUnit>("DSP", "Follow");
	m_unitFactory->addUnitPrototype<syn::DCRemoverUnit>("DSP", "DC Trap");

	m_unitFactory->addUnitPrototype<syn::ConstantUnit>("Math", "Const");
	m_unitFactory->addUnitPrototype<syn::SummerUnit>("Math", "Sum");
	m_unitFactory->addUnitPrototype<syn::GainUnit>("Math", "Gain");
	m_unitFactory->addUnitPrototype<syn::LerpUnit>("Math", "Affine");
	m_unitFactory->addUnitPrototype<syn::RectifierUnit>("Math", "Rect");
	m_unitFactory->addUnitPrototype<syn::PitchToFreqUnit>("Math", "Pitch2Freq");
	m_unitFactory->addUnitPrototype<syn::FreqToPitchUnit>("Math", "Freq2Pitch");

	m_unitFactory->addUnitPrototype<syn::TanhUnit>("Waveshapers", "tanh");

	m_unitFactory->addUnitPrototype<syn::GateUnit>("MIDI", "Gate");
	m_unitFactory->addUnitPrototype<syn::MidiNoteUnit>("MIDI", "Pitch");
	m_unitFactory->addUnitPrototype<syn::VelocityUnit>("MIDI", "Vel");
	m_unitFactory->addUnitPrototype<syn::MidiCCUnit>("MIDI", "CC");

	m_unitFactory->addUnitPrototype<synui::OscilloscopeUnit>("Visualizer", "Oscilloscope");
	m_unitFactory->addUnitPrototype<synui::SpectroscopeUnit>("Visualizer", "Spectroscope");

	m_unitFactory->addUnitPrototype<syn::InputUnit>("", "Input");
	m_unitFactory->addUnitPrototype<syn::OutputUnit>("", "Output");

	m_voiceManager = new syn::VoiceManager(m_unitFactory);
	m_voiceManager->setMaxVoices(8);

	m_MIDIReceiver = new syn::MIDIReceiver(m_voiceManager);
}

VOSIMSynth::~VOSIMSynth() {
	boost::checked_delete(m_voiceManager);
	boost::checked_delete(m_unitFactory);
	boost::checked_delete(m_MIDIReceiver);
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
	shared_ptr<syn::Unit> circuit(m_voiceManager->getPrototypeCircuit()->clone());
	stringstream ss; {
		cereal::JSONOutputArchive ar(ss);
		ar(cereal::make_nvp("circuit", circuit));
	}
	pChunk->PutStr(ss.str().c_str());

	getVOSIMComponent()->save(pChunk);
	return true;
}

int VOSIMSynth::UnserializeState(ByteChunk* pChunk, int startPos) {
	m_unitFactory->resetBuildCounts();

	string input;
	startPos = pChunk->Get(&input, startPos);
	stringstream ss{ input };
	shared_ptr<syn::Unit> circuit;
	{
		cereal::JSONInputArchive ar(ss);
		ar(cereal::make_nvp("circuit", circuit));
	}
	getVOSIMComponent()->circuitPanel()->reset();
	m_voiceManager->setPrototypeCircuit(*static_cast<const syn::Circuit*>(circuit.get()));
	Reset();
	startPos = getVOSIMComponent()->load(pChunk, startPos);
	return startPos;
}

void VOSIMSynth::PresetsChangedByHost() {}

void VOSIMSynth::OnIdle() {
	m_voiceManager->onIdle();
}

void VOSIMSynth::OnActivate(bool active) {}

void VOSIMSynth::OnGUIOpen() {}

void VOSIMSynth::OnGUIClose() {}

synui::VOSIMComponent* VOSIMSynth::getVOSIMComponent() const {
	return static_cast<synui::VOSIMComponent*>(GetAppWindow()->getRoot());
}

void VOSIMSynth::Reset() {
	m_MIDIReceiver->Resize(GetBlockSize());
	m_voiceManager->setBufferSize(GetBlockSize());
	m_voiceManager->setFs(GetSampleRate());
}

void VOSIMSynth::OnParamChange(int paramIdx) {}