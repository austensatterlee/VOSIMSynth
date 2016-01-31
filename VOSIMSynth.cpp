#include <stdlib.h>
#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"
#include "EnvelopeEditor.h"
#include "VosimOscillator.h"
#include "ADSREnvelope.h"
#include "MathUnits.h"

using namespace std;

const int kNumPrograms = 1;

enum EParams
{
	kMainVol = 0,
};

enum ELayout
{
	kWidth = GUI_WIDTH,
	kHeight = GUI_HEIGHT,
	kColorKnobFrames = 27,
	kNumberedKnobFrames = 101
};

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
	:
	IPLUG_CTOR(256, kNumPrograms, instanceInfo) {
	TRACE;

	//MakePreset("preset 1", ... );
	//MakeDefaultPreset((char *) "-", kNumPrograms);

	makeInstrument();
	makeGraphics();
}

void VOSIMSynth::makeGraphics() {
	pGraphics = MakeGraphics(this, kWidth, kHeight);
	pGraphics->HandleMouseOver(true);
	IColor bg_color = COLOR_BLACK;
	pGraphics->AttachPanelBackground(&bg_color);

	// IBitmap wedgeswitch2p = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
	// IBitmap push2p = pGraphics->LoadIBitmap(PUSH_2P_ID, PUSH_2P_FN, 2);
	// IBitmap colorKnob = pGraphics->LoadIBitmap(COLOR_RING_KNOB_ID, COLOR_RING_KNOB_FN, kColorKnobFrames);
	// IBitmap toggleswitch3p = pGraphics->LoadIBitmap(TOGGLE_SWITCH_3P_ID, TOGGLE_SWITCH_3P_FN, 3);
	// IBitmap numberedKnob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kNumberedKnobFrames);

	m_Oscilloscope = new Oscilloscope(this, IRECT(5, kHeight - 200, kWidth - 5, kHeight - 5), &m_voiceManager);
	pGraphics->AttachControl(m_Oscilloscope);

	m_circuitPanel = new CircuitPanel(this, {5,5,kWidth - 5,kHeight - 210}, &m_voiceManager, m_unitfactory);
	pGraphics->AttachControl(m_circuitPanel);

	AttachGraphics(pGraphics);
}

void VOSIMSynth::makeInstrument() {
	m_instr = new Instrument();
	m_unitfactory = new UnitFactory();
	m_unitfactory->addUnitPrototype( "Generators", new VosimOscillator("VOSIM") );
	m_unitfactory->addUnitPrototype( "Generators", new BasicOscillator("Basic") );
	m_unitfactory->addUnitPrototype( "Generators", new FormantOscillator("Formant"));
	m_unitfactory->addUnitPrototype( "Modulators",new Envelope("Envelope") );
	m_unitfactory->addUnitPrototype( "Modulators",new ADSREnvelope("ADSREnvelope") );
	m_unitfactory->addUnitPrototype( "Modulators", new LFOOscillator("LFO") );
	m_unitfactory->addUnitPrototype( "Math",new PanningUnit("Panner") );
	m_unitfactory->addUnitPrototype( "Math", new RectifierUnit("Rectifier") );
	m_unitfactory->addUnitPrototype( "Math", new InvertingUnit("Inverter"));
	//m_unitfactory->addUnitPrototype( "Math", new MemoryUnit("Memory"));

	m_voiceManager.setMaxVoices(16, m_instr);

	m_MIDIReceiver.noteOn.Connect(&m_voiceManager, &VoiceManager::noteOn);
	m_MIDIReceiver.noteOff.Connect(&m_voiceManager, &VoiceManager::noteOff);
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
	// Mutex is already locked for us.
	for (int s = 0; s < nFrames; s++) {
		m_MIDIReceiver.advance();
		m_sampleCount++;
	}
	memset(outputs[0], 0, nFrames*sizeof(double));
	memset(outputs[1], 0, nFrames*sizeof(double));
	m_voiceManager.onHostReset(GetSampleRate(), GetBlockSize(), GetTempo());
	m_voiceManager.tick(outputs, nFrames);
	m_Oscilloscope->process();
	m_MIDIReceiver.Flush(nFrames);
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg) {
	IMutexLock lock(this);
	m_MIDIReceiver.onMessageReceived(pMsg);
}

void VOSIMSynth::SetInstrParameter(int unitid, int paramid, double value) {
	int paramidx = m_invHostParamMap[unitid][paramid];
	GetParam(paramidx)->Set(value);
	OnParamChange(paramidx);
}

double VOSIMSynth::GetInstrParameter(int unitid, int paramid) {
	int paramidx = m_invHostParamMap[unitid][paramid];
	return GetParam(paramidx)->Value();
}

bool VOSIMSynth::SerializeState(ByteChunk* pChunk) {
	IMutexLock lock(this);
	ByteChunk serialized = m_circuitPanel->serialize();
	pChunk->PutChunk(&serialized);
	return true;
}

int VOSIMSynth::UnserializeState(ByteChunk* pChunk, int startPos) {
	IMutexLock lock(this);
	startPos = m_circuitPanel->unserialize(pChunk, startPos);
	return startPos;
}

void VOSIMSynth::PresetsChangedByHost() {
	IMutexLock lock(this);
}

void VOSIMSynth::Reset() {
	TRACE;
	IMutexLock lock(this);
	m_MIDIReceiver.Resize(GetBlockSize());
	m_voiceManager.onHostReset(GetSampleRate(), GetBlockSize(), GetTempo());
}

void VOSIMSynth::OnParamChange(int paramIdx) {
	IMutexLock lock(this);
	if (paramIdx < m_hostParamMap.size()) {
		int uid = m_hostParamMap[paramIdx].first;
		int pid = m_hostParamMap[paramIdx].second;
		m_voiceManager.modifyParameter(uid, pid, GetParam(paramIdx)->Value(), SET);
		InformHostOfParamChange(paramIdx, GetParam(paramIdx)->Value());
	}
}

