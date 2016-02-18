#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"
#include "Oscillator.h"
#include "VosimOscillator.h"
#include "ADSREnvelope.h"
#include "MathUnits.h"
#include "include/OscilloscopeUnit.h"

using namespace std;

const int kNumPrograms = 1;

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
        : IPLUG_CTOR(0, kNumPrograms, instanceInfo)
{
    TRACE;

    //MakePreset("preset 1", ... );
    //MakeDefaultPreset((char *) "-", kNumPrograms);

    makeInstrument();
    makeGraphics();
}

void VOSIMSynth::makeGraphics()
{
    pGraphics = MakeGraphics(this, GUI_WIDTH, GUI_HEIGHT);
    pGraphics->HandleMouseOver(true);
    IColor bg_color = COLOR_BLACK;
    pGraphics->AttachPanelBackground(&bg_color);

    // IBitmap wedgeswitch2p = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
    // IBitmap push2p = pGraphics->LoadIBitmap(PUSH_2P_ID, PUSH_2P_FN, 2);
    // IBitmap colorKnob = pGraphics->LoadIBitmap(COLOR_RING_KNOB_ID, COLOR_RING_KNOB_FN, kColorKnobFrames);
    // IBitmap toggleswitch3p = pGraphics->LoadIBitmap(TOGGLE_SWITCH_3P_ID, TOGGLE_SWITCH_3P_FN, 3);
    // IBitmap numberedKnob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kNumberedKnobFrames);

    m_circuitPanel = make_shared<CircuitPanel>(this, IRECT{5, 5, GUI_WIDTH - 5, GUI_HEIGHT - 5}, m_voiceManager, m_unitFactory);

	m_circuitPanel->registerUnitControl(make_shared<OscilloscopeUnit>(""), make_shared<OscilloscopeUnitControl>());

    pGraphics->AttachControl(m_circuitPanel.get());

    AttachGraphics(pGraphics);
}

void VOSIMSynth::makeInstrument()
{
	shared_ptr<Circuit> circ = make_shared<Circuit>("main");
    m_unitFactory = make_shared<UnitFactory>();
    m_unitFactory->addUnitPrototype("Oscillators", new VosimOscillator("VOSIM"));
    m_unitFactory->addUnitPrototype("Oscillators", new BasicOscillator("Basic"));
    m_unitFactory->addUnitPrototype("Oscillators", new FormantOscillator("Formant"));

    m_unitFactory->addUnitPrototype("Modulators", new ADSREnvelope("ADSREnvelope"));
	m_unitFactory->addUnitPrototype("Modulators", new LFOOscillator("LFO"));

    m_unitFactory->addUnitPrototype("DSP", new HalfRectifierUnit("Half Rectifier"));
	m_unitFactory->addUnitPrototype("DSP", new FullRectifierUnit("Full Rectifier"));
    m_unitFactory->addUnitPrototype("DSP", new InvertingUnit("Inverter"));
	m_unitFactory->addUnitPrototype("DSP", new MemoryUnit("Memory"));
	m_unitFactory->addUnitPrototype("DSP", new GainUnit("Gain"));

	m_unitFactory->addUnitPrototype("Math", new SummerUnit("Summer"));
	m_unitFactory->addUnitPrototype("Math", new MultiplyUnit("Multiplier"));
	m_unitFactory->addUnitPrototype("Math", new ConstantUnit("Constant"));

	m_unitFactory->addUnitPrototype("MIDI", new GateUnit("Gate"));
    m_unitFactory->addUnitPrototype("MIDI", new MidiNoteUnit("Pitch"));
	m_unitFactory->addUnitPrototype("MIDI", new NormalizedMidiNoteUnit("Normalized Pitch"));
	m_unitFactory->addUnitPrototype("MIDI", new VelocityUnit("Velocity"));

	m_unitFactory->addUnitPrototype("Visualizer", new OscilloscopeUnit("Oscilloscope"));

    m_voiceManager = make_shared<VoiceManager>(circ, m_unitFactory);
    m_voiceManager->setMaxVoices(16);

    m_MIDIReceiver = make_shared<MIDIReceiver>(shared_ptr<VoiceManager>(m_voiceManager));
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
    // Mutex is already locked for us.
    for (int s = 0 ; s < nFrames ; s++) {
        m_MIDIReceiver->advance();
    }
    //memset(outputs[0], 0, nFrames * sizeof(double));
    //memset(outputs[1], 0, nFrames * sizeof(double));
    m_voiceManager->setTempo(GetTempo());
	
    for (int i = 0 ; i < nFrames ; i++) {
        m_voiceManager->tick(inputs[0][i],inputs[1][i],outputs[0][i],outputs[1][i]);
    }
    m_MIDIReceiver->Flush(nFrames);
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg)
{
    IMutexLock lock(this);
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
    startPos = m_circuitPanel->unserialize(pChunk, startPos);
    return startPos;
}

void VOSIMSynth::PresetsChangedByHost()
{
    IMutexLock lock(this);
}

void VOSIMSynth::Reset()
{
    TRACE;
    IMutexLock lock(this);
    m_MIDIReceiver->Resize(GetBlockSize());
    m_voiceManager->setFs(GetSampleRate());
}

void VOSIMSynth::OnParamChange(int paramIdx)
{
    IMutexLock lock(this);
}
