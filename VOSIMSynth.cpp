#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"
#include <cmath>
#include <ctime>
#include <tuple>

using namespace std;
#define O_U "vosc"

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
  m_Oscilloscope(this, IRECT(10, 310, 800 - 10, 600 - 10), 1000),
  m_EnvEditor(this, IRECT(600, 10, 800 - 10, 90), 4)
{
  TRACE;

  //const ParamProp realParams[kNumParams] = {
  //    TypedParamProp<double>("Gain",1E-3,setGain,[](double a,double b) -> const double { 20 * pow(10,2 * a - 1); }),
  //    TypedParamProp<double>("Vol A",1E-3,mOutGain,[](double a,double b) -> const double { 20 * pow(10,2 * a - 1); }),
  //    TypedParamProp<double>("Vol D")
  //};

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kMainVol)->InitDouble("Main Vol", 0.5, 0.0, 1.0, 0.001, "Main Vol");
  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);

  makeInstrument();
  makeGraphics();
}

void VOSIMSynth::makeGraphics()
{


  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_BLACK);

  //IBitmap wedgeswitch2p = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
  IBitmap push2p = pGraphics->LoadIBitmap(PUSH_2P_ID, PUSH_2P_FN, 2);
  IBitmap colorKnob = pGraphics->LoadIBitmap(COLOR_RING_KNOB_ID, COLOR_RING_KNOB_FN, kColorKnobFrames);
  //IBitmap toggleswitch3p = pGraphics->LoadIBitmap(TOGGLE_SWITCH_3P_ID, TOGGLE_SWITCH_3P_FN, 3);
  IBitmap numberedKnob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kNumberedKnobFrames);

  //attachKnob(pGraphics, this, 0, 0, kMainVol, &numberedKnob);

  pGraphics->AttachControl(&m_Oscilloscope);
  pGraphics->AttachControl(&m_EnvEditor);

  vector<tuple<string, string>> pnames = m_instr->getParameterNames();
  int i = 0;
  for (tuple<string, string> splitname : pnames)
  {
    string name = get<0>(splitname) + "," + get<1>(splitname);
    GetParam(i)->InitDouble(get<1>(splitname).c_str(), 0, 0, 1, 1E-3, name.c_str(), get<0>(splitname).c_str());
    attachKnob(pGraphics, this, i * 2 / 8, (i * 2) % 8, i, &numberedKnob);
    i++;
  }
  m_numParameters = i;

  AttachGraphics(pGraphics);
}

void VOSIMSynth::makeInstrument()
{
  Envelope* env = new Envelope();
  env->setPeriod(0, 0.1, 1);
  env->setPeriod(2, 1, 1);
  m_instr = new Instrument();
  m_instr->addSource("env", env);
  m_instr->addSource("vosc", new VosimOscillator());
  m_instr->addUnit("summer", new AccumulatingUnit());
  m_instr->addUnit("modwheel", new AccumulatingUnit());
  m_instr->addConnection(new Connection("env", "vosc", "pitch", ADD));
  m_instr->addConnection(new Connection("env", "vosc", "gain", SCALE));
  m_instr->addConnection(new Connection{ "vosc","summer","input",ADD });
  m_instr->addConnection(new Connection{ "modwheel","vosc","pitch", SC });
  m_instr->modifyParameter("modwheel", "gain", SET, 12.0);
  m_instr->addMIDIConnection(new MIDIConnection{ IMidiMsg::EControlChangeMsg::kModWheel,"modwheel","input",SET });
  m_instr->setSink("summer");

  m_voiceManager.setInstrument(m_instr);
  m_voiceManager.setMaxVoices(16);
  m_voiceManager.m_onDyingVoice.Connect(this, &VOSIMSynth::OnDyingVoice);

  m_MIDIReceiver.noteOn.Connect(this, &VOSIMSynth::OnNoteOn);
  m_MIDIReceiver.noteOff.Connect(this, &VOSIMSynth::OnNoteOff);
  m_MIDIReceiver.sendControlChange.Connect(&m_voiceManager, &VoiceManager::sendMIDICC);
}

void VOSIMSynth::OnNoteOn(uint8_t pitch, uint8_t vel)
{
  IMutexLock lock(this);
  m_voiceManager.noteOn(pitch, vel);
  Instrument* vnew = m_voiceManager.getNewestVoice();
  if (vnew)
  {
    m_Oscilloscope.connectInput(vnew->getSourceUnit(O_U));
    m_Oscilloscope.connectTrigger(vnew->getSourceUnit(O_U));
  }
}

void VOSIMSynth::OnNoteOff(uint8_t pitch, uint8_t vel)
{
  IMutexLock lock(this);
  Instrument* vold = m_voiceManager.noteOff(pitch, vel);
  //m_Oscilloscope.disconnectInput(vold->getSourceUnit(O_U));
  //m_Oscilloscope.disconnectTrigger(vold->getSourceUnit(O_U));
  //Instrument* vnew = m_voiceManager.getNewestVoice();
  //if (vnew)
  //{
  //  m_Oscilloscope.connectInput(vnew->getSourceUnit(O_U));
  //  m_Oscilloscope.connectTrigger(vnew->getSourceUnit(O_U));
  //}
}

void VOSIMSynth::OnDyingVoice(Instrument* dying)
{
  m_Oscilloscope.disconnectInput(dying->getSourceUnit(O_U));
  m_Oscilloscope.disconnectTrigger(dying->getSourceUnit(O_U));
  Instrument* vnew = m_voiceManager.getNewestVoice();
  if (vnew)
  {
    m_Oscilloscope.connectInput(vnew->getSourceUnit(O_U));
    m_Oscilloscope.connectTrigger(vnew->getSourceUnit(O_U));
  }
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];
  for (int s = 0; s < nFrames; s++)
  {
    leftOutput[s] = 0;
    m_MIDIReceiver.advance();
    leftOutput[s] = m_voiceManager.tick();
    rightOutput[s] = leftOutput[s];
    mLastOutput = leftOutput[s];
  }

  m_MIDIReceiver.Flush(nFrames);
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg)
{
  IMutexLock lock(this);
  m_MIDIReceiver.onMessageReceived(pMsg);
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
    string pname = GetParam(paramIdx)->GetNameForHost();
    string uname = GetParam(paramIdx)->GetParamGroupForHost();
    m_voiceManager.modifyParameter(uname, pname, SET, GetParam(paramIdx)->Value());
  }
}
