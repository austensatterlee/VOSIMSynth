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
#include "MathUnits.h"
#include "include/OscilloscopeUnit.h"
#include "Follower.h"
#include "MemoryUnit.h"
#include "MidiUnits.h"
#include "StateVariableFilter.h"
#include "fft.h"
#include "include/SpectroscopeUnit.h"
#include "VOSIMWindow.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"
#include "UnitFactory.h"

using namespace std;

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(0, 0, instanceInfo), m_tempo(0) {
	TRACE;

	WDL_fft_init();
	makeInstrument();
	makeGraphics();
}

void VOSIMSynth::makeGraphics() {
	VOSIMWindow* vosimWindow = new VOSIMWindow(GUI_WIDTH, GUI_HEIGHT, m_voiceManager, m_unitFactory);
	vosimWindow->setHInstance(gHInstance);
	AttachAppWindow(vosimWindow);

	vosimWindow->registerUnitControl<OscilloscopeUnit>([](VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId)-> UIUnitControl* {
		return new OscilloscopeUnitControl(a_window, a_vm, a_unitId);
	});
	vosimWindow->registerUnitControl<SpectroscopeUnit>([](VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId)-> UIUnitControl* {
		return new SpectroscopeUnitControl(a_window, a_vm, a_unitId);
	});
}

void VOSIMSynth::makeInstrument() {
	m_unitFactory = new UnitFactory();
	m_unitFactory->addUnitPrototype<StateVariableFilter>("Filters","SVF");
	m_unitFactory->addUnitPrototype<LagUnit>("Filters","Lag");

	m_unitFactory->addUnitPrototype<BasicOscillator>("Oscillators","Basic");
	m_unitFactory->addUnitPrototype<VosimOscillator>("Oscillators","VOSIM");
	m_unitFactory->addUnitPrototype<FormantOscillator>("Oscillators","Formant");

	m_unitFactory->addUnitPrototype<ADSREnvelope>("Modulators","ADSR");
	m_unitFactory->addUnitPrototype<LFOOscillator>("Modulators","LFO");

	m_unitFactory->addUnitPrototype<MemoryUnit>("DSP","Delay");
	m_unitFactory->addUnitPrototype<ResampleUnit>("DSP","Delay2");
	m_unitFactory->addUnitPrototype<PanningUnit>("DSP","Pan");
	m_unitFactory->addUnitPrototype<FollowerUnit>("DSP","Follow");
	m_unitFactory->addUnitPrototype<DCRemoverUnit>("DSP","DC Trap");

	m_unitFactory->addUnitPrototype<ConstantUnit>("Math","Const");
	m_unitFactory->addUnitPrototype<SummerUnit>("Math","Sum");
	m_unitFactory->addUnitPrototype<GainUnit>("Math","Gain");
	m_unitFactory->addUnitPrototype<MACUnit>("Math","MAC");
	m_unitFactory->addUnitPrototype<LerpUnit>("Math","Affine");
	m_unitFactory->addUnitPrototype<RectifierUnit>("Math","Rect");

	m_unitFactory->addUnitPrototype<GateUnit>("MIDI","Gate");
	m_unitFactory->addUnitPrototype<MidiNoteUnit>("MIDI","CV");
	m_unitFactory->addUnitPrototype<VelocityUnit>("MIDI","Vel");
	m_unitFactory->addUnitPrototype<MidiCCUnit>("MIDI","CC");

	m_unitFactory->addUnitPrototype<OscilloscopeUnit>("Visualizer","Oscilloscope");
	m_unitFactory->addUnitPrototype<SpectroscopeUnit>("Visualizer","Spectroscope");

	m_unitFactory->addUnitPrototype<PassthroughUnit>("","Passthrough");

	m_voiceManager = new VoiceManager(m_unitFactory);
	m_voiceManager->setMaxVoices(6);

	m_MIDIReceiver = new MIDIReceiver(m_voiceManager);
}

VOSIMSynth::~VOSIMSynth() {
	if (m_voiceManager)
		DELETE_NULL(m_voiceManager);
	if (m_unitFactory)
		DELETE_NULL(m_unitFactory);
	if (m_MIDIReceiver)
		DELETE_NULL(m_MIDIReceiver);
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
	if (GetAppWindow()->isInitialized()) {
		m_voiceManager->save(pChunk);
		GetAppWindow()->save(pChunk);
		return true;
	}
	return false;
}

int VOSIMSynth::UnserializeState(ByteChunk* pChunk, int startPos) {
	if (GetAppWindow()->isInitialized()) {
		m_unitFactory->resetBuildCounts();
		GetAppWindow()->reset();
		startPos = m_voiceManager->load(pChunk, startPos);
		startPos = GetAppWindow()->load(pChunk, startPos);
	}
	return startPos;
}

void VOSIMSynth::PresetsChangedByHost() {}

void VOSIMSynth::OnIdle() {
	m_voiceManager->onIdle();
}

void VOSIMSynth::OnActivate(bool active) {}

void VOSIMSynth::OnGUIOpen() {}

void VOSIMSynth::OnGUIClose() { }

bool VOSIMSynth::isTransportRunning() {
	GetTime(&m_timeInfo);
	return m_timeInfo.mTransportIsRunning;
}

void VOSIMSynth::Reset() {
	m_MIDIReceiver->Resize(GetBlockSize());
	m_voiceManager->setBufferSize(GetBlockSize());
	m_voiceManager->setFs(GetSampleRate());
}

void VOSIMSynth::OnParamChange(int paramIdx) {}