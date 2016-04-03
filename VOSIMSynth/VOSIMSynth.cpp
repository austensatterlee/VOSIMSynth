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
#include "KeyboardControl.h"

using namespace std;

const int kNumPrograms = 1;

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
        : IPLUG_CTOR(0, kNumPrograms, instanceInfo),
	m_tempo(0)
{
    TRACE;

    //MakePreset("preset 1", ... );
    //MakeDefaultPreset((char *) "-", kNumPrograms);

	WDL_fft_init();
    makeInstrument();
    makeGraphics();
}

void VOSIMSynth::makeGraphics()
{
    m_graphics = shared_ptr<IGraphics>(MakeGraphics(this, GUI_WIDTH, GUI_HEIGHT));
    m_graphics->HandleMouseOver(true);
	m_graphics->SetStrictDrawing(true);
    IColor bg_color = COLOR_BLACK;
    m_graphics->AttachPanelBackground(&bg_color);
	
	m_kbdctrl = make_shared<syn::KeyboardControl>(this, m_voiceManager, IRECT{ 1,GUI_HEIGHT - 100,GUI_WIDTH - 1,GUI_HEIGHT - 1 });
	m_graphics->AttachControl(m_kbdctrl.get());

    // IBitmap wedgeswitch2p = m_graphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
    // IBitmap push2p = m_graphics->LoadIBitmap(PUSH_2P_ID, PUSH_2P_FN, 2);
    // IBitmap colorKnob = m_graphics->LoadIBitmap(COLOR_RING_KNOB_ID, COLOR_RING_KNOB_FN, kColorKnobFrames);
    // IBitmap toggleswitch3p = m_graphics->LoadIBitmap(TOGGLE_SWITCH_3P_ID, TOGGLE_SWITCH_3P_FN, 3);
    // IBitmap numberedKnob = m_graphics->LoadIBitmap(KNOB_ID, KNOB_FN, kNumberedKnobFrames);

    m_circuitPanel = make_shared<CircuitPanel>(this, IRECT{1, 1, GUI_WIDTH - 1, GUI_HEIGHT - 100}, m_voiceManager.get(), m_unitFactory.get());

	m_circuitPanel->registerUnitControl<OscilloscopeUnit, OscilloscopeUnitControl>();
	m_circuitPanel->registerUnitControl<SpectroscopeUnit, SpectroscopeUnitControl>();

    m_graphics->AttachControl(m_circuitPanel.get());

    AttachGraphics(m_graphics.get());
}

void VOSIMSynth::makeInstrument()
{
	shared_ptr<Circuit> circ = make_shared<Circuit>("main");
    m_unitFactory = make_shared<UnitFactory>();
	m_unitFactory->addUnitPrototype("Filters", new StateVariableFilter("SVF"));
	m_unitFactory->addUnitPrototype("Filters", new LagUnit("Lag"));

    m_unitFactory->addUnitPrototype("Oscillators", new VosimOscillator("VOSIM"));
    m_unitFactory->addUnitPrototype("Oscillators", new BasicOscillator("Basic"));
    m_unitFactory->addUnitPrototype("Oscillators", new FormantOscillator("Formant"));

    m_unitFactory->addUnitPrototype("Modulators", new ADSREnvelope("ADSR"));
	m_unitFactory->addUnitPrototype("Modulators", new LFOOscillator("LFO"));

    m_unitFactory->addUnitPrototype("DSP", new HalfRectifierUnit("H. Rect"));
	m_unitFactory->addUnitPrototype("DSP", new FullRectifierUnit("F. Rect"));
	m_unitFactory->addUnitPrototype("DSP", new MemoryUnit("Memory"));
	m_unitFactory->addUnitPrototype("DSP", new PanningUnit("Pan"));
	m_unitFactory->addUnitPrototype("DSP", new FollowerUnit("Follow"));
	m_unitFactory->addUnitPrototype("DSP", new DCRemoverUnit("DC Trap"));

	m_unitFactory->addUnitPrototype("Math", new SummerUnit("Sum"));
	m_unitFactory->addUnitPrototype("Math", new GainUnit("Gain"));
	m_unitFactory->addUnitPrototype("Math", new MACUnit("MAC"));
	m_unitFactory->addUnitPrototype("Math", new InvertingUnit("Inv"));
	m_unitFactory->addUnitPrototype("Math", new LinToDbUnit("Lin2dB"));
	m_unitFactory->addUnitPrototype("Math", new LerpUnit("Affine"));
	m_unitFactory->addUnitPrototype("Math", new ConstantUnit("Const"));

	m_unitFactory->addUnitPrototype("MIDI", new GateUnit("Gate"));
    m_unitFactory->addUnitPrototype("MIDI", new MidiNoteUnit("Pitch"));
	m_unitFactory->addUnitPrototype("MIDI", new VelocityUnit("Vel"));
	m_unitFactory->addUnitPrototype("MIDI", new MidiCCUnit("CC"));

	m_unitFactory->addUnitPrototype("Visualizer", new OscilloscopeUnit("Oscilloscope"));
	m_unitFactory->addUnitPrototype("Visualizer", new SpectroscopeUnit("Spectroscope"));

    m_voiceManager = make_shared<VoiceManager>(circ, m_unitFactory);
    m_voiceManager->setMaxVoices(6);

    m_MIDIReceiver = make_shared<MIDIReceiver>(shared_ptr<VoiceManager>(m_voiceManager));
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
    // Mutex is already locked for us.

    for (int s = 0 ; s < nFrames ; s++) {
        m_MIDIReceiver->advance();
    }

	// If tempo has changed, notify instrument
	if (m_tempo != GetTempo()) {
		m_voiceManager->setTempo(GetTempo());
	}
	
	// Process samples
    m_voiceManager->tick(inputs[0],inputs[1],outputs[0],outputs[1]);

    m_MIDIReceiver->Flush(nFrames);
	m_tickCount++;
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg)
{
    m_MIDIReceiver->onMessageReceived(pMsg);
}

bool VOSIMSynth::SerializeState(ByteChunk* pChunk)
{
    ByteChunk serialized = m_circuitPanel->serialize();
    pChunk->PutChunk(&serialized);
    return true;
}

int VOSIMSynth::UnserializeState(ByteChunk* pChunk, int startPos)
{
	m_unitFactory->resetBuildCounts();
    startPos = m_circuitPanel->unserialize(pChunk, startPos);
    return startPos;
}

void VOSIMSynth::PresetsChangedByHost()
{
}

void VOSIMSynth::OnIdle() {
}

void VOSIMSynth::OnActivate(bool active) {
}

bool VOSIMSynth::isTransportRunning() {
	GetTime(&m_timeInfo);
	return m_timeInfo.mTransportIsRunning;
}

void VOSIMSynth::Reset()
{
    m_MIDIReceiver->Resize(GetBlockSize());
	m_voiceManager->setBufferSize(GetBlockSize());
    m_voiceManager->setFs(GetSampleRate());
}

void VOSIMSynth::OnParamChange(int paramIdx)
{
}

