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
#define O_TU "bosc0"
#define O_U "master"


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
  IPLUG_CTOR(256, kNumPrograms, instanceInfo)
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
  pGraphics->AttachPanelBackground(&pt2color(palette[0]));

  //IBitmap wedgeswitch2p = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
  IBitmap push2p = pGraphics->LoadIBitmap(PUSH_2P_ID, PUSH_2P_FN, 2);
  IBitmap colorKnob = pGraphics->LoadIBitmap(COLOR_RING_KNOB_ID, COLOR_RING_KNOB_FN, kColorKnobFrames);
  //IBitmap toggleswitch3p = pGraphics->LoadIBitmap(TOGGLE_SWITCH_3P_ID, TOGGLE_SWITCH_3P_FN, 3);
  IBitmap numberedKnob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kNumberedKnobFrames);

  // add parameters from instrument
  vector<tuple<string, string>> pnames = m_instr->getParameterNames();
  int i = 0;
  for (tuple<string, string> splitname : pnames)
  {
    string name = get<0>(splitname) + "-" + get<1>(splitname);
    int ownerid = m_instr->getUnitId(get<0>(splitname));
    Unit& owner = m_instr->getUnit(ownerid);
    int paramid = owner.getParamId(get<1>(splitname));
    UnitParameter& param = owner.getParam(paramid);

    param.initIParam(GetParam(i));

    m_hostParamMap.push_back(make_pair(ownerid, paramid));
    m_invHostParamMap[ownerid][paramid] = i;
    i++;
  }
  m_numParameters = i;
  
  m_Oscilloscope = new Oscilloscope(this, IRECT(10, 600, 790, 790));
  EnvelopeEditor* m_EnvEditor1 = new EnvelopeEditor(this, &m_voiceManager, "ampenv1", IRECT(500, 10, 800 - 10, 150), 10, -1.0, 1.0, 1);
  EnvelopeEditor* m_EnvEditor2 = new EnvelopeEditor(this, &m_voiceManager, "ampenv2", IRECT(500, 160, 800 - 10, 300), 10, -1.0, 1.0, 1);
  EnvelopeEditor* m_EnvEditor3 = new EnvelopeEditor(this, &m_voiceManager, "pulseenv1", IRECT(500, 310, 800 - 10, 450), 10, -1.0, 1.0, 1);
  EnvelopeEditor* m_EnvEditor4 = new EnvelopeEditor(this, &m_voiceManager, "decayenv1", IRECT(500, 460, 800 - 10, 600), 10, -1, 1, 1);
 
  pGraphics->AttachControl(m_Oscilloscope);
  pGraphics->AttachControl(m_EnvEditor1);
  pGraphics->AttachControl(m_EnvEditor2);
  pGraphics->AttachControl(m_EnvEditor3);
  pGraphics->AttachControl(m_EnvEditor4);

  int j = 0;
  i = 0;
  for (; i < m_hostParamMap.size(); i++)
  {
    string ownername = m_instr->getUnit(m_hostParamMap[i].first).getName();
    UnitParameter& param = m_instr->getUnit(m_hostParamMap[i].first).getParam(m_hostParamMap[i].second);
    string name = ownername + "." + param.getName();
    if (!param.isHidden() && !param.hasController())
    {
      if (param.getType() == DOUBLE_TYPE)
      {
        attachKnob<IKnobMultiControl>(pGraphics, this, j * 2 / 10, (j * 2) % 10, i, name, &numberedKnob);
      }
      else  if (param.getType() == INT_TYPE || param.getType() == ENUM_TYPE)
      {
        attachKnob<IKnobMultiControl>(pGraphics, this, j * 2 / 10, (j * 2) % 10, i, name, &colorKnob);
      }
      else  if (param.getType() == BOOL_TYPE)
      {
        attachKnob<ISwitchControl>(pGraphics, this, j * 2 / 10, (j * 2) % 10, i, name, &push2p);
      }
      j++;
    }
  }

  AttachGraphics(pGraphics);
}

void VOSIMSynth::makeInstrument()
{
  Envelope* env1 = new Envelope("ampenv1");
  Envelope* env2 = new Envelope("ampenv2");
  Envelope* env3 = new Envelope("pulseenv1");
  Envelope* env4 = new Envelope("decayenv1");
  VosimChoir* vchoir0 = new VosimChoir("vchoir0",7);
  Oscillator* osc0 = new VosimOscillator("osc0");
  Oscillator* bosc0 = new BasicOscillator("bosc0");
  AccumulatingUnit* acc = new AccumulatingUnit("master");
  Filter<AA_FILTER_SIZE+1, AA_FILTER_SIZE>* filt1 = new Filter<AA_FILTER_SIZE+1, AA_FILTER_SIZE>("filt1",AA_FILTER_X, AA_FILTER_Y);

  m_instr = new Instrument();
  m_instr->addSource(env1);
  m_instr->addSource(env2);
  m_instr->addSource(env3);
  m_instr->addSource(env4);
  m_instr->addSource(vchoir0);
  m_instr->addSource(bosc0);
  m_instr->addUnit(acc);
  m_instr->addUnit(filt1);
  m_instr->addConnection("ampenv1", "bosc0", "gain", SCALE);
  m_instr->addConnection("ampenv2", "vchoir0", "gain", SCALE);
  m_instr->addConnection("pulseenv1", "vchoir0", "pulsepitch", ADD);
  m_instr->addConnection("decayenv1", "vchoir0", "decay", SCALE);
  m_instr->addConnection("vchoir0", "master", "input", ADD);
  m_instr->addConnection("bosc0", "master", "input", ADD);
  m_instr->addConnection("master", "filt1", "input", ADD);

  m_instr->setSinkName("master");
  m_instr->setPrimarySource("ampenv1");
  m_instr->addPrimarySource("ampenv2");

  m_voiceManager.setMaxVoices(16, m_instr);
  m_instr->getUnit("master").modifyParameter(1, 1. / m_voiceManager.getMaxVoices(), SET);
  m_voiceManager.m_onDyingVoice.Connect(this, &VOSIMSynth::OnDyingVoice);

  m_MIDIReceiver.noteOn.Connect(this, &VOSIMSynth::OnNoteOn);
  m_MIDIReceiver.noteOff.Connect(&m_voiceManager, &VoiceManager::noteOff);
}

void VOSIMSynth::unitTickHook(double process)
{}

void VOSIMSynth::OnNoteOn(uint8_t pitch, uint8_t vel)
{
  m_voiceManager.noteOn(pitch, vel);
  Instrument* vnew = m_voiceManager.getNewestVoice();
  if (vnew)
  {
    m_Oscilloscope->connectInput(vnew->getUnit(O_U));
    m_Oscilloscope->connectTrigger(vnew->getSourceUnit(O_TU));
  }
}

void VOSIMSynth::OnDyingVoice(Instrument* dying)
{
  Instrument* vnew = m_voiceManager.getNewestVoice();
  if (vnew)
  {
    m_Oscilloscope->connectInput(vnew->getUnit(O_U));
    m_Oscilloscope->connectTrigger(vnew->getSourceUnit(O_TU));
  }
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];
  memset(leftOutput,0,nFrames*sizeof(double));
  for (int s = 0; s < nFrames; s++)
  {
    m_MIDIReceiver.advance();
    m_sampleCount++;
  }
  m_voiceManager.tick(leftOutput,nFrames);
  memcpy(rightOutput,leftOutput,nFrames*sizeof(double));
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
  if (paramIdx < m_numParameters)
  {
    int uid = m_hostParamMap[paramIdx].first;
    int pid = m_hostParamMap[paramIdx].second;
    m_voiceManager.modifyParameter(uid, pid, GetParam(paramIdx)->Value(), SET);
    InformHostOfParamChange(paramIdx, GetParam(paramIdx)->Value());
  }
}
