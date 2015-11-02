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
#define O_TU "vosc2"
#define O_U "filt1"


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
  m_Oscilloscope(this, IRECT(10, 500, 790, 800), 1000)
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
  pGraphics->AttachPanelBackground(&IColor(255,0,5,0));

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

    if(param.getType()==DOUBLE_TYPE){
      GetParam(i)->InitDouble(get<1>(splitname).c_str(), param.getBase(), param.getMin(), param.getMax(), 1E-3, name.c_str(), name.c_str());
    }
    else if (param.getType() == INT_TYPE)
    {
      GetParam(i)->InitInt(get<1>(splitname).c_str(), param.getBase(), param.getMin(), param.getMax(), name.c_str(), name.c_str());
    }
    else if (param.getType() == BOOL_TYPE)
    {
      GetParam(i)->InitBool(get<1>(splitname).c_str(), param.getBase(), name.c_str());
    }
  
    m_hostParamMap.push_back(make_pair(ownerid, paramid));
    m_invHostParamMap[ownerid][paramid] = i;
    i++;
  }
  m_numParameters = i;

  EnvelopeEditor* m_EnvEditor1 = new EnvelopeEditor(this, &m_voiceManager, "ampenv1", IRECT(500, 10, 800 - 10, 150), 10, -1.0, 1.0);
  EnvelopeEditor* m_EnvEditor2 = new EnvelopeEditor(this, &m_voiceManager, "ampenv2", IRECT(500, 160, 800 - 10, 300), 10, -1.0, 1.0);
  EnvelopeEditor* m_EnvEditor3 = new EnvelopeEditor(this, &m_voiceManager, "vampenv", IRECT(500, 310, 800 - 10, 450), 10, -1.0, 1.0);
  EnvelopeEditor* m_EnvEditor4 = new EnvelopeEditor(this, &m_voiceManager, "vrateenv", IRECT(200, 310, 500 - 10, 450), 10, 0, 1);

  pGraphics->AttachControl(&m_Oscilloscope);
  pGraphics->AttachControl(m_EnvEditor1);
  pGraphics->AttachControl(m_EnvEditor2);
  pGraphics->AttachControl(m_EnvEditor3);
  pGraphics->AttachControl(m_EnvEditor4);

  m_Oscilloscope.setTransformFunc(Oscilloscope::magnitudeTransform);
  m_Oscilloscope.usePeriodEstimate(false);
  m_Oscilloscope.setDefaultBufSize(128);
  //m_Oscilloscope.setTransformFunc(Oscilloscope::passthruTransform);
  
  int j = 0;
  i=0;
  for (; i < m_hostParamMap.size(); i++)
  {
    string ownername = m_instr->getUnit(m_hostParamMap[i].first).getName();
    UnitParameter& param = m_instr->getUnit(m_hostParamMap[i].first).getParam(m_hostParamMap[i].second);
    string name = ownername+"."+param.getName();
    if (!param.isHidden() && !param.hasController())
    {
      if(param.getType()==DOUBLE_TYPE){
        attachKnob(pGraphics, this, j * 2 / 8, (j * 2) % 8, i, name, &numberedKnob);
      }
      else  if (param.getType() == INT_TYPE)
      {
        attachKnob(pGraphics, this, j * 2 / 8, (j * 2) % 8, i, name, &colorKnob);
      }
      else  if (param.getType() == BOOL_TYPE)
      {
        attachSwitch(pGraphics, this, j * 2 / 8, (j * 2) % 8, i, name, &push2p);
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
  Envelope* env3 = new Envelope("vampenv");
  Envelope* env4 = new Envelope("vrateenv");
  LFOOscillator* lfo0 = new LFOOscillator("lfo0");
  lfo0->getParam("waveform").mod(SQUARE_WAVE, SET);
  Oscillator* vosc1 = new VosimOscillator("vosc1");
  Oscillator* vosc2 = new VosimOscillator("vosc2");
  AccumulatingUnit* acc = new AccumulatingUnit("master");
  Filter<AA_FILTER_SIZE + 1, AA_FILTER_SIZE>* filt1 = new Filter<AA_FILTER_SIZE + 1, AA_FILTER_SIZE>("filt1", AA_FILTER_X, AA_FILTER_Y);

  m_instr = new Instrument();
  m_instr->addSource(env1);
  m_instr->addSource(env2);
  m_instr->addSource(env3);
  m_instr->addSource(env4);
  m_instr->addSource(lfo0);
  m_instr->addSource(vosc1);
  m_instr->addSource(vosc2);
  m_instr->addUnit(filt1);
  m_instr->addUnit(acc);
  m_instr->addConnection("ampenv1", "vosc1", "gain", SCALE);
  m_instr->addConnection("ampenv2", "vosc2", "gain", SCALE);
  m_instr->addConnection("vampenv", "lfo0", "gain", SCALE);
  m_instr->addConnection("vrateenv", "lfo0", "pitchshift", ADD);
  m_instr->addConnection("lfo0", "vosc2", "pitchshift", ADD);
  m_instr->addConnection("vosc1", "vosc2", "phaseshift", ADD);
  m_instr->addConnection("vosc2", "master", "input", ADD);
  m_instr->addConnection("master", "filt1", "input", ADD);
  m_instr->addMIDIConnection(IMidiMsg::EControlChangeMsg::kModWheel,"lfo0","gain",SCALE);

  m_instr->setSinkName("filt1");
  m_instr->setPrimarySource("ampenv2");

  m_voiceManager.setMaxVoices(8, m_instr);
  m_instr->getUnit("master").modifyParameter(1, 1. / m_voiceManager.getMaxVoices(), SET);
  m_voiceManager.m_onDyingVoice.Connect(this, &VOSIMSynth::OnDyingVoice);

  m_MIDIReceiver.noteOn.Connect(this, &VOSIMSynth::OnNoteOn);
  m_MIDIReceiver.noteOff.Connect(&m_voiceManager, &VoiceManager::noteOff);
  m_MIDIReceiver.sendControlChange.Connect(&m_voiceManager, &VoiceManager::sendMIDICC);
}

void VOSIMSynth::unitTickHook(double input)
{}

void VOSIMSynth::OnNoteOn(uint8_t pitch, uint8_t vel)
{
  m_voiceManager.noteOn(pitch, vel);
  Instrument* vnew = m_voiceManager.getNewestVoice();
  if (vnew)
  {
    m_Oscilloscope.connectInput(vnew->getUnit(O_U));
    m_Oscilloscope.connectTrigger(vnew->getSourceUnit(O_TU));
  }
}

void VOSIMSynth::OnDyingVoice(Instrument* dying)
{
  Instrument* vnew = m_voiceManager.getNewestVoice();
  if (vnew)
  {
    m_Oscilloscope.connectInput(vnew->getUnit(O_U));
    m_Oscilloscope.connectTrigger(vnew->getSourceUnit(O_TU));
  }
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];
  for (int s = 0; s < nFrames; s++)
  {
    m_MIDIReceiver.advance();
    leftOutput[s] = rightOutput[s] = m_voiceManager.tick();
    m_sampleCount++;
  }
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
  InformHostOfParamChange(paramidx, GetParam(paramidx)->GetNormalized());
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
  m_voiceManager.setFs(fs);
}

void VOSIMSynth::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  int n = m_voiceManager.getMaxVoices();

  if (paramIdx < m_numParameters)
  {
    int uid = m_hostParamMap[paramIdx].first;
    int pid = m_hostParamMap[paramIdx].second;
    m_voiceManager.modifyParameter(uid, pid, GetParam(paramIdx)->Value(), SET);
  }
}
