#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"
#include "EnvelopeEditor.h"
#include "VosimOscillator.h"
#include "IControl.h"
#include "UI.h"
#include "Filter.h"
#include <cmath>
#include <ctime>
#include <tuple>
#include <cstring>

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
  IPLUG_CTOR(256, kNumPrograms, instanceInfo),
  m_oscilloscope_input_unit(-1),
  m_oscilloscope_trigger_unit(-1)
{
  TRACE;
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

  //MakePreset("preset 1", ... );
  //MakeDefaultPreset((char *) "-", kNumPrograms);

  makeInstrument();
  makeGraphics();
}

void VOSIMSynth::makeGraphics()
{
  pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&(IColor)(globalPalette[0]));

  //IBitmap wedgeswitch2p = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
  IBitmap push2p = pGraphics->LoadIBitmap(PUSH_2P_ID, PUSH_2P_FN, 2);
  IBitmap colorKnob = pGraphics->LoadIBitmap(COLOR_RING_KNOB_ID, COLOR_RING_KNOB_FN, kColorKnobFrames);
  //IBitmap toggleswitch3p = pGraphics->LoadIBitmap(TOGGLE_SWITCH_3P_ID, TOGGLE_SWITCH_3P_FN, 3);
  IBitmap numberedKnob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kNumberedKnobFrames);

  m_Oscilloscope = new Oscilloscope(this, IRECT(10, 600, 790, 790));
  pGraphics->AttachControl(m_Oscilloscope);

  m_circuitPanel = new CircuitPanel(this, { 5,5,795,550 }, &m_voiceManager, m_unitfactory);
  pGraphics->AttachControl(m_circuitPanel);

  AttachGraphics(pGraphics);
}

void VOSIMSynth::makeInstrument()
{
  m_instr = new Instrument();
  m_unitfactory = new UnitFactory();
  m_unitfactory->addSourceUnitPrototype(new Envelope("Envelope"));
  m_unitfactory->addUnitPrototype(new AccumulatingUnit("Accumulator"));
  m_unitfactory->addSourceUnitPrototype(new VosimOscillator("Osc.VOSIM"));
  m_unitfactory->addSourceUnitPrototype(new VosimChoir("Osc.VOSIM.Choir"));
  //m_unitfactory->addSourceUnitPrototype(new NormalRandomOscillator("Osc.Random.Normal"));
  m_unitfactory->addSourceUnitPrototype(new BasicOscillator("Osc.Basic"));
  m_unitfactory->addSourceUnitPrototype(new LFOOscillator("Osc.LFO"));

  m_voiceManager.setMaxVoices(4, m_instr);

  m_voiceManager.m_onDyingVoice.Connect(this, &VOSIMSynth::OnDyingVoice);
  m_MIDIReceiver.noteOn.Connect(this, &VOSIMSynth::OnNoteOn);
  m_MIDIReceiver.noteOff.Connect(&m_voiceManager, &VoiceManager::noteOff);
}

void VOSIMSynth::OnNoteOn(uint8_t pitch, uint8_t vel)
{
  m_voiceManager.noteOn(pitch, vel);
  Instrument* vnew = m_voiceManager.getNewestVoice();
  if (vnew && m_oscilloscope_input_unit != -1 && m_oscilloscope_trigger_unit != -1)
  {
    m_Oscilloscope->connectInput(vnew->getUnit(m_oscilloscope_input_unit));
    m_Oscilloscope->connectTrigger(vnew->getSourceUnit(m_oscilloscope_trigger_unit));
  }
}

void VOSIMSynth::OnDyingVoice(Instrument* dying)
{
  Instrument* vnew = m_voiceManager.getNewestVoice();
  if (vnew && m_oscilloscope_input_unit != -1 && m_oscilloscope_trigger_unit != -1)
  {
    m_Oscilloscope->connectInput(vnew->getUnit(m_oscilloscope_input_unit));
    m_Oscilloscope->connectTrigger(vnew->getSourceUnit(m_oscilloscope_trigger_unit));
  }
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];
  memset(leftOutput, 0, nFrames*sizeof(double));
  for (int s = 0; s < nFrames; s++)
  {
    m_MIDIReceiver.advance();
    m_sampleCount++;
  }
  m_voiceManager.tick(leftOutput, nFrames);
  memcpy(rightOutput, leftOutput, nFrames*sizeof(double));
  m_Oscilloscope->process();
  m_MIDIReceiver.Flush(nFrames);
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg)
{
  IMutexLock lock(this);
  m_MIDIReceiver.onMessageReceived(pMsg);
}

void VOSIMSynth::SetInstrParameter(int unitid, int paramid, double value)
{
  int paramidx = m_invHostParamMap[unitid][paramid];
  GetParam(paramidx)->Set(value);
  OnParamChange(paramidx);
}

double VOSIMSynth::GetInstrParameter(int unitid, int paramid)
{
  int paramidx = m_invHostParamMap[unitid][paramid];
  return GetParam(paramidx)->Value();
}

bool VOSIMSynth::SerializeState(ByteChunk* pChunk)
{
  IMutexLock lock(this);
  ByteChunk serialized = m_circuitPanel->serialize();
  pChunk->PutChunk(&serialized);
  return true;
}

int VOSIMSynth::UnserializeState(ByteChunk* pChunk, int startPos)
{
  IMutexLock lock(this);
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
  double fs = GetSampleRate();
  m_MIDIReceiver.Resize(GetBlockSize());
  m_voiceManager.setBufSize(GetBlockSize());
  m_voiceManager.setFs(fs);
}

void VOSIMSynth::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  if (paramIdx < m_hostParamMap.size())
  {
    int uid = m_hostParamMap[paramIdx].first;
    int pid = m_hostParamMap[paramIdx].second;
    m_voiceManager.modifyParameter(uid, pid, GetParam(paramIdx)->Value(), SET);
    InformHostOfParamChange(paramIdx, GetParam(paramIdx)->Value());
  }
}