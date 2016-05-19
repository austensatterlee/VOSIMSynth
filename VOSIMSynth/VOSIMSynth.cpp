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
	m_unitFactory->addUnitPrototype("Filters", new StateVariableFilter("SVF"));
	m_unitFactory->addUnitPrototype("Filters", new LagUnit("Lag"));

	m_unitFactory->addUnitPrototype("Oscillators", new BasicOscillator("Basic"));
	m_unitFactory->addUnitPrototype("Oscillators", new VosimOscillator("VOSIM"));
	m_unitFactory->addUnitPrototype("Oscillators", new FormantOscillator("Formant"));

	m_unitFactory->addUnitPrototype("Modulators", new ADSREnvelope("ADSR"));
	m_unitFactory->addUnitPrototype("Modulators", new LFOOscillator("LFO"));

	m_unitFactory->addUnitPrototype("DSP", new MemoryUnit("Memory"));
	m_unitFactory->addUnitPrototype("DSP", new PanningUnit("Pan"));
	m_unitFactory->addUnitPrototype("DSP", new FollowerUnit("Follow"));
	m_unitFactory->addUnitPrototype("DSP", new DCRemoverUnit("DC Trap"));

	m_unitFactory->addUnitPrototype("Math", new ConstantUnit("Const"));
	m_unitFactory->addUnitPrototype("Math", new SummerUnit("Sum"));
	m_unitFactory->addUnitPrototype("Math", new GainUnit("Gain"));
	m_unitFactory->addUnitPrototype("Math", new MACUnit("MAC"));
	m_unitFactory->addUnitPrototype("Math", new LerpUnit("Affine"));
	m_unitFactory->addUnitPrototype("Math", new RectifierUnit("Rect"));

	m_unitFactory->addUnitPrototype("MIDI", new GateUnit("Gate"));
	m_unitFactory->addUnitPrototype("MIDI", new MidiNoteUnit("Pitch"));
	m_unitFactory->addUnitPrototype("MIDI", new VelocityUnit("Vel"));
	m_unitFactory->addUnitPrototype("MIDI", new MidiCCUnit("CC"));

	m_unitFactory->addUnitPrototype("Visualizer", new OscilloscopeUnit("Oscilloscope"));
	m_unitFactory->addUnitPrototype("Visualizer", new SpectroscopeUnit("Spectroscope"));

	m_unitFactory->addUnitPrototype("", new PassthroughUnit("Passthrough"));

	m_voiceManager = new VoiceManager(make_shared<Circuit>("main"), m_unitFactory);
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

		//		ActionMessage* msg = new ActionMessage();
		//		msg->action = [](Circuit* a_circ, bool a_isLast, ByteChunk* a_data)
		//		{
		//			if (a_isLast) {
		//				int* chunkStartPos;
		//				ByteChunk* chunk;
		//				VoiceManager* vm;
		//				VOSIMWindow* vw;
		//
		//				int localStartPos = 0;
		//				localStartPos = a_data->Get<int*>(&chunkStartPos, localStartPos);
		//				localStartPos = a_data->Get<ByteChunk*>(&chunk, localStartPos);
		//				localStartPos = a_data->Get<VoiceManager*>(&vm, localStartPos);
		//				localStartPos = a_data->Get<VOSIMWindow*>(&vw, localStartPos);
		//
		//				*chunkStartPos = vm->load(chunk, *chunkStartPos);
		//				*chunkStartPos = vw->load(chunk, *chunkStartPos);
		//			}
		//		};
		//		int* startPosAddr = &startPos;
		//		msg->data.Put<int*>(&startPosAddr);
		//		msg->data.Put<ByteChunk*>(&pChunk);
		//		msg->data.Put<VoiceManager*>(&m_voiceManager);
		//		msg->data.Put<VOSIMWindow*>(&m_vosimWindow);
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
