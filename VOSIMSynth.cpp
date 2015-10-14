#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"
#include <cmath>
#include <ctime>

using namespace std;

const int kNumPrograms = 1;

enum EParams {
  kMainVol = 0,
  // Volume envelope
  kEnv1Atk,
  kEnv1Dec,
  kEnv1Rel,
  kEnv1Sus,
  kEnv1Shp,
  // Pitch LFO
  kGlobalPMF,
  kGlobalPMG,
  // Oscillator 1 params
  kOsc1Tune,
  kOsc1VF,
  kOsc1Num,
  kOsc1Dec,
  kOsc1Vol,
  kOsc1FreqMode,
  kEnv2Int,
  kEnv2Atk,
  kEnv2Dec,
  kEnv2Rel,
  kEnv2Sus,
  kEnv2Shp,
  // Oscillator 2 params
  kOsc2Tune,
  kOsc2VF,
  kOsc2Num,
  kOsc2Dec,
  kOsc2Vol,
  kOsc2FreqMode,
  kEnv3Int,
  kEnv3Atk,
  kEnv3Dec,
  kEnv3Rel,
  kEnv3Sus,
  kEnv3Shp,
  // Oscillator 3 params
  kOsc3Tune,
  kOsc3VF,
  kOsc3Num,
  kOsc3Dec,
  kOsc3Vol,
  kOsc3FreqMode,
  kEnv4Int,
  kEnv4Atk,
  kEnv4Dec,
  kEnv4Rel,
  kEnv4Sus,
  kEnv4Shp,
  kNumParams
};

enum ELayout {
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,
  kColorKnobFrames = 27,
  kNumberedKnobFrames = 101
};

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
  :
  IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
  m_Oscilloscope(this, IRECT(10, 310, 800 - 10, 600 - 10), 1000),
  m_EnvEditor(this, IRECT(600,10,800-10,90),4)  
  {
  TRACE;

  //const ParamProp realParams[kNumParams] = {
  //    TypedParamProp<double>("Gain",1E-3,setGain,[](double a,double b) -> const double { 20 * pow(10,2 * a - 1); }),
  //    TypedParamProp<double>("Vol A",1E-3,mOutGain,[](double a,double b) -> const double { 20 * pow(10,2 * a - 1); }),
  //    TypedParamProp<double>("Vol D")
  //};

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kMainVol)->InitDouble("Main Vol", 0.5, 0.0, 1.0, 0.001, "Main Vol");


  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_BLACK);

  //IBitmap wedgeswitch2p = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
  IBitmap push2p = pGraphics->LoadIBitmap(PUSH_2P_ID, PUSH_2P_FN, 2);
  IBitmap colorKnob = pGraphics->LoadIBitmap(COLOR_RING_KNOB_ID, COLOR_RING_KNOB_FN, kColorKnobFrames);
  //IBitmap toggleswitch3p = pGraphics->LoadIBitmap(TOGGLE_SWITCH_3P_ID, TOGGLE_SWITCH_3P_FN, 3);
  IBitmap numberedKnob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kNumberedKnobFrames);

  attachKnob(pGraphics, this, 0, 8, kMainVol, &numberedKnob);

  pGraphics->AttachControl(&m_Oscilloscope);
  pGraphics->AttachControl(&m_EnvEditor);

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);

  Envelope* env = new Envelope();
  env->setPeriod(0,0.1,1);
  env->setPeriod(2,1,1);
  m_instr.addSource("env", env);
  m_instr.addUnit("vosc", new Oscillator());
  m_instr.addUnit("summer",new AccumulatingUnit());
  m_instr.addConnection(new Connection("env", "vosc", "pitch", SCALE));
  m_instr.addConnection(new Connection("env", "vosc", "gain", SCALE));
  m_instr.addConnection(new Connection{ "vosc","summer","input",ADD});
  m_instr.setSink("summer");
  // m_MIDIReceiver.sendControlChange.Connect(&m_instr,&Instrument::sendMIDICC);
  m_MIDIReceiver.noteOn.Connect(this, &VOSIMSynth::OnNoteOn);
  m_MIDIReceiver.noteOff.Connect(this, &VOSIMSynth::OnNoteOff);
  m_voiceManager.setInstrument(&m_instr);
  m_voiceManager.setMaxVoices(3);

}

void VOSIMSynth::OnNoteOn(uint8_t pitch, uint8_t vel) {
  IMutexLock lock(this);
  m_voiceManager.noteOn(pitch,vel);
}

void VOSIMSynth::OnNoteOff(uint8_t pitch, uint8_t vel) {
  IMutexLock lock(this);
  m_voiceManager.noteOff(pitch, vel);
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
  // Mutex is already locked for us.
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];
  for (int s = 0; s < nFrames; s++) {
    leftOutput[s] = 0;
    m_MIDIReceiver.advance();
    leftOutput[s] = mOutGain*m_voiceManager.tick();
    rightOutput[s] = leftOutput[s];
    mLastOutput = leftOutput[s];
    m_Oscilloscope.input(mLastOutput);    
  }

  m_MIDIReceiver.Flush(nFrames);
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg) {
  IMutexLock lock(this);
  m_MIDIReceiver.onMessageReceived(pMsg);
}

void VOSIMSynth::Reset() {
  TRACE;
  IMutexLock lock(this);
  double fs = GetSampleRate();
  m_voiceManager.setFs(fs);
}

void VOSIMSynth::OnParamChange(int paramIdx) {
  IMutexLock lock(this);
  int n = m_voiceManager.getMaxVoices();
  switch (paramIdx) {
  case kMainVol:
    mOutGain = dbToAmp(LERP(ampToDb(1./n), 0, GetParam(kMainVol)->Value()));
    break;
  default:
    break;
  }
}
