#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include "UI.h"
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


class ParamProp {
public:
  ParamProp(const char *name)
    : mName(name) {};
private:
  const char *mName;
};

/*
A parameter describes the values that a piece of memory can assume, and it provides a way to map the range (0,1) onto a new memory value.
Each parameter needs to know its target memory location, its initial value, and a function y_[n+1]=f(x,y[n]), where x is a double in (0,1).
 */
template <class T>
class TypedParamProp : public ParamProp {
public:
  TypedParamProp(const char *name, const T defVal, T &target, const T(*modify)(T, T)) :
    ParamProp(name),
    mDefault(defVal),
    mTarget(target),
    mModify(modify) {
    if (is_integral<T>::value)
      mStep = 1;
    else
      mStep = 1E-3;
  };
private:
  const T mDefault;
  const T mStep;
  T &mTarget;
  const T(*mModify)(T, T);
};

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
  : IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo) {
  TRACE;

  //const ParamProp realParams[kNumParams] = {
  //    TypedParamProp<double>("Gain",1E-3,setGain,[](double a,double b) -> const double { 20 * pow(10,2 * a - 1); }),
  //    TypedParamProp<double>("Vol A",1E-3,mOutGain,[](double a,double b) -> const double { 20 * pow(10,2 * a - 1); }),
  //    TypedParamProp<double>("Vol D")
  //};

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kMainVol)->InitDouble("Main Vol", 0.5, 0.0, 1.0, 0.001, "Main Vol");

  GetParam(kEnv1Atk)->InitDouble("Vol A", 0.0, 0.0, 1.0, 0.001, "Vol A");
  GetParam(kEnv1Dec)->InitDouble("Vol D", 0.1, 0.0, 1.0, 0.001, "Vol D");
  GetParam(kEnv1Rel)->InitDouble("Vol R", 0.0, 0.0, 1.0, 0.001, "Vol R");
  GetParam(kEnv1Sus)->InitDouble("Vol S", 0.8, 0.0, 1.0, 0.001, "Vol S");
  GetParam(kEnv1Shp)->InitDouble("Vol Shp", 0.5, 0.0, 1.0, 0.001, "Vol Shp");

  GetParam(kEnv2Int)->InitDouble("Osc1 VF Int", 0.0, -1.0, 1.0, 0.001, "Osc1 VF Int");
  GetParam(kEnv2Atk)->InitDouble("Osc1 VF A", 0.0, 0.0, 1.0, 0.001, "Osc1 VF A");
  GetParam(kEnv2Dec)->InitDouble("Osc1 VF D", 0.1, 0.0, 1.0, 0.001, "Osc1 VF D");
  GetParam(kEnv2Rel)->InitDouble("Osc1 VF R", 0.1, 0.0, 1.0, 0.001, "Osc1 VF R");
  GetParam(kEnv2Sus)->InitDouble("Osc1 VF S", 0.1, 0.0, 1.0, 0.001, "Osc1 VF S");
  GetParam(kEnv2Shp)->InitDouble("Osc1 VF Shp", 0.5, 0.0, 1.0, 0.001, "Osc1 VF Shp");

  GetParam(kEnv3Int)->InitDouble("Osc2 VF Int", 0.0, -1.0, 1.0, 0.001, "Osc2 VF Int");
  GetParam(kEnv3Atk)->InitDouble("Osc2 VF A", 0.0, 0.0, 1.0, 0.001, "Osc2 VF A");
  GetParam(kEnv3Dec)->InitDouble("Osc2 VF D", 0.1, 0.0, 1.0, 0.001, "Osc2 VF D");
  GetParam(kEnv3Rel)->InitDouble("Osc2 VF R", 0.1, 0.0, 1.0, 0.001, "Osc2 VF R");
  GetParam(kEnv3Sus)->InitDouble("Osc2 VF S", 0.1, 0.0, 1.0, 0.001, "Osc2 VF S");
  GetParam(kEnv3Shp)->InitDouble("Osc2 VF Shp", 0.5, 0.0, 1.0, 0.001, "Osc2 VF Shp");

  GetParam(kEnv4Int)->InitDouble("Osc2 VF Int", 0.0, -1.0, 1.0, 0.001, "Osc3 VF Int");
  GetParam(kEnv4Atk)->InitDouble("Osc3 VF A", 0.0, 0.0, 1.0, 0.001, "Osc3 VF A");
  GetParam(kEnv4Dec)->InitDouble("Osc3 VF D", 0.1, 0.0, 1.0, 0.001, "Osc3 VF D");
  GetParam(kEnv4Rel)->InitDouble("Osc3 VF R", 0.1, 0.0, 1.0, 0.001, "Osc3 VF R");
  GetParam(kEnv4Sus)->InitDouble("Osc3 VF S", 0.1, 0.0, 1.0, 0.001, "Osc3 VF S");
  GetParam(kEnv4Shp)->InitDouble("Osc3 VF Shp", 0.5, 0.0, 1.0, 0.001, "Osc3 VF Shp");

  GetParam(kGlobalPMF)->InitDouble("OPMf", 0.001, 0.0, 1.0, 0.001, "Pitch Mod F");
  GetParam(kGlobalPMG)->InitDouble("OPMg", 0.001, 0.0, 1.0, 0.001, "Pitch Mod G");

  GetParam(kOsc1Tune)->InitInt("O1Tune", 0, -36., 36., "Osc1 Tune");
  GetParam(kOsc1VF)->InitDouble("O1VF", 0.5, 0, 1, 0.000001, "Osc1 VF");
  GetParam(kOsc1Num)->InitDouble("O1N", 0.25, 0, 1, 0.000001, "Osc1 Number");
  GetParam(kOsc1Dec)->InitDouble("O1D", 0.001, 0.0, 1.0, 0.000001, "Osc1 Decay");
  GetParam(kOsc1Vol)->InitDouble("O1Vol", 1, 0, 1, 0.001, "Osc1 Vol");
  GetParam(kOsc1FreqMode)->InitBool("O1RelFreq", true, "Osc1 Freq Mode", "Osc1");

  GetParam(kOsc2Tune)->InitInt("O2Tune", 0.0, -36., 36., "Osc2 Tune");
  GetParam(kOsc2VF)->InitDouble("O2VF", 0.5, 0, 1, 0.000001, "Osc2 VF");
  GetParam(kOsc2Num)->InitDouble("O2N", 0.25, 0, 1, 0.000001, "Osc2 Number");
  GetParam(kOsc2Dec)->InitDouble("O2D", 0.001, 0.0, 1.0, 0.000001, "Osc2 Decay");
  GetParam(kOsc2Vol)->InitDouble("O2Vol", 1, 0, 1, 0.001, "Osc2 Vol");
  GetParam(kOsc2FreqMode)->InitBool("O2RelFreq", true, "Osc2 Freq Mode", "Osc2");

  GetParam(kOsc3Tune)->InitInt("O3Tune", 0.0, -36., 36., "Osc3 Tune");
  GetParam(kOsc3VF)->InitDouble("O3VF", 0.5, 0, 1, 0.000001, "Osc3 VF", "Osc3");
  GetParam(kOsc3Num)->InitDouble("O3N", 0.25, 0, 1, 0.000001, "Osc3 Number", "Osc3");
  GetParam(kOsc3Dec)->InitDouble("O3D", 0.001, 0.0, 1.0, 0.000001, "Osc3 Decay", "Osc3");
  GetParam(kOsc3Vol)->InitDouble("O3Vol", 1, 0, 1, 0.001, "Osc3 Vol", "Osc3");
  GetParam(kOsc3FreqMode)->InitBool("O3RelFreq", true, "Osc3 Freq Mode", "Osc3");


  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_BLACK);

  //IBitmap wedgeswitch2p = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
  IBitmap push2p = pGraphics->LoadIBitmap(PUSH_2P_ID, PUSH_2P_FN, 2);
  IBitmap colorKnob = pGraphics->LoadIBitmap(COLOR_RING_KNOB_ID, COLOR_RING_KNOB_FN, kColorKnobFrames);
  //IBitmap toggleswitch3p = pGraphics->LoadIBitmap(TOGGLE_SWITCH_3P_ID, TOGGLE_SWITCH_3P_FN, 3);
  IBitmap numberedKnob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kNumberedKnobFrames);

  attachKnob(pGraphics, this, 0, 8, kMainVol, &numberedKnob);

  attachKnob(pGraphics, this, 0, 1, kEnv1Atk, &numberedKnob);
  attachKnob(pGraphics, this, 0, 2, kEnv1Dec, &numberedKnob);
  attachKnob(pGraphics, this, 0, 4, kEnv1Rel, &numberedKnob);
  attachKnob(pGraphics, this, 0, 3, kEnv1Sus, &numberedKnob);
  attachKnob(pGraphics, this, 0, 0, kEnv1Shp, &numberedKnob);

  attachKnob(pGraphics, this, 5, 0, kEnv2Int, &numberedKnob);
  attachKnob(pGraphics, this, 1, 1, kEnv2Atk, &numberedKnob);
  attachKnob(pGraphics, this, 1, 2, kEnv2Dec, &numberedKnob);
  attachKnob(pGraphics, this, 1, 4, kEnv2Rel, &numberedKnob);
  attachKnob(pGraphics, this, 1, 3, kEnv2Sus, &numberedKnob);
  attachKnob(pGraphics, this, 1, 0, kEnv2Shp, &numberedKnob);

  attachKnob(pGraphics, this, 5, 1, kEnv3Int, &numberedKnob);
  attachKnob(pGraphics, this, 2, 1, kEnv3Atk, &numberedKnob);
  attachKnob(pGraphics, this, 2, 2, kEnv3Dec, &numberedKnob);
  attachKnob(pGraphics, this, 2, 4, kEnv3Rel, &numberedKnob);
  attachKnob(pGraphics, this, 2, 3, kEnv3Sus, &numberedKnob);
  attachKnob(pGraphics, this, 2, 0, kEnv3Shp, &numberedKnob);

  attachKnob(pGraphics, this, 5, 2, kEnv4Int, &numberedKnob);
  attachKnob(pGraphics, this, 3, 1, kEnv4Atk, &numberedKnob);
  attachKnob(pGraphics, this, 3, 2, kEnv4Dec, &numberedKnob);
  attachKnob(pGraphics, this, 3, 4, kEnv4Rel, &numberedKnob);
  attachKnob(pGraphics, this, 3, 3, kEnv4Sus, &numberedKnob);
  attachKnob(pGraphics, this, 3, 0, kEnv4Shp, &numberedKnob);

  attachKnob(pGraphics, this, 0, 9, kGlobalPMF, &numberedKnob);
  attachKnob(pGraphics, this, 0, 10, kGlobalPMG, &numberedKnob);

  attachKnob(pGraphics, this, 4, 0, kOsc1Tune, &colorKnob);
  attachKnob(pGraphics, this, 1, 5, kOsc1VF, &numberedKnob);
  attachKnob(pGraphics, this, 1, 6, kOsc1Num, &numberedKnob);
  attachKnob(pGraphics, this, 1, 7, kOsc1Dec, &numberedKnob);
  attachKnob(pGraphics, this, 1, 8, kOsc1Vol, &numberedKnob);
  attachSwitch(pGraphics, this, 1, 9, kOsc1FreqMode, &push2p);


  attachKnob(pGraphics, this, 4, 1, kOsc2Tune, &colorKnob);
  attachKnob(pGraphics, this, 2, 5, kOsc2VF, &numberedKnob);
  attachKnob(pGraphics, this, 2, 6, kOsc2Num, &numberedKnob);
  attachKnob(pGraphics, this, 2, 7, kOsc2Dec, &numberedKnob);
  attachKnob(pGraphics, this, 2, 8, kOsc2Vol, &numberedKnob);
  attachSwitch(pGraphics, this, 2, 9, kOsc2FreqMode, &push2p);

  attachKnob(pGraphics, this, 4, 2, kOsc3Tune, &colorKnob);
  attachKnob(pGraphics, this, 3, 5, kOsc3VF, &numberedKnob);
  attachKnob(pGraphics, this, 3, 6, kOsc3Num, &numberedKnob);
  attachKnob(pGraphics, this, 3, 7, kOsc3Dec, &numberedKnob);
  attachKnob(pGraphics, this, 3, 8, kOsc3Vol, &numberedKnob);
  attachSwitch(pGraphics, this, 3, 9, kOsc3FreqMode, &push2p);

  mOscilloscope = new Oscilloscope(this, IRECT(10, 310, 800 - 10, 600 - 10), 1000);
  mOscilloscope->setNumDisplayPeriods(2);
  pGraphics->AttachControl(mOscilloscope);

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
  mVoiceManager.setNumVoices(NUM_VOICES);
  mMIDIReceiver.noteOn.Connect(this, &VOSIMSynth::OnNoteOn);
  mMIDIReceiver.noteOff.Connect(this, &VOSIMSynth::OnNoteOff);
  double xcoefs[7] = { 4.760635e-1, 2.856281, 7.140952, 9.521270, 7.140952, 2.856281, 4.760635e-1 };
  double ycoefs[6] = { -4.522403, -8.676844, -9.007512, -5.328429, -1.702543, -2.303303e-1 };
  LP4 = new Filter(xcoefs, ycoefs, 7, 6);
}

void VOSIMSynth::OnNoteOn(uint8_t pitch, uint8_t vel) {
  mVoiceManager.TriggerNote(pitch, vel);
  Voice& v = mVoiceManager.getLowestVoice();
  mOscilloscope->connectTrigger(&v);
  mOscilloscope->connectInput(LP4);
}

void VOSIMSynth::OnNoteOff(uint8_t pitch, uint8_t vel) {
  Voice& v = mVoiceManager.ReleaseNote(pitch, vel);
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
  // Mutex is already locked for us.
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];
  for (int s = 0; s < nFrames; s++) {
    leftOutput[s] = 0;
    mMIDIReceiver.advance();
    leftOutput[s] = LP4->process(mVoiceManager.process());
    rightOutput[s] = leftOutput[s];
    mLastOutput = leftOutput[s];
  }

  mMIDIReceiver.Flush(nFrames);
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg) {
  mMIDIReceiver.onMessageReceived(pMsg);
}

void VOSIMSynth::Reset() {
  TRACE;
  IMutexLock lock(this);
  double fs = GetSampleRate();
  mVoiceManager.setFs(fs);
}

void VOSIMSynth::OnParamChange(int paramIdx) {
  IMutexLock lock(this);
  uint8_t n = mVoiceManager.getNumVoices();
  switch (paramIdx) {
  case kMainVol:
    LP4->m_pGain.set(pow(10, 0.05*LERP(-18, 36, GetParam(kMainVol)->Value())));
    LP4->freezeParams();
    break;
    /* Env 1 */
  case kEnv1Atk:
    while (n--)
      mVoiceManager.m_voices[n].mAmpEnv.setPeriod(0, GetParam(kEnv1Atk)->Value(), 0);
    break;
  case kEnv1Dec:
    while (n--)
      mVoiceManager.m_voices[n].mAmpEnv.setPeriod(1, GetParam(kEnv1Dec)->Value(), 0);
    break;
  case kEnv1Rel:
    while (n--)
      mVoiceManager.m_voices[n].mAmpEnv.setPeriod(2, GetParam(kEnv1Rel)->Value(), 0);
    break;
  case kEnv1Sus:
    while (n--)
      mVoiceManager.m_voices[n].mAmpEnv.setPoint(1, GetParam(kEnv1Sus)->Value());
    break;
  case kEnv1Shp:
    while (n--)
      mVoiceManager.m_voices[n].mAmpEnv.setShape(-1, GetParam(kEnv1Shp)->Value());
    break;
    /* Env 2 */
  case kEnv2Int:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[0].m_pGain.set(127 *GetParam(kEnv2Int)->Value());
    break;
  case kEnv2Atk:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[0].setPeriod(0, GetParam(kEnv2Atk)->Value(), 0);
    break;
  case kEnv2Dec:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[0].setPeriod(1, GetParam(kEnv2Dec)->Value(), 0);
    break;
  case kEnv2Rel:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[0].setPeriod(2, GetParam(kEnv2Rel)->Value(), 0);
    break;
  case kEnv2Sus:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[0].setPoint(1, GetParam(kEnv2Sus)->Value());
    break;
  case kEnv2Shp:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[0].setShape(-1, GetParam(kEnv2Shp)->Value());
    break;
    /* Env 3*/
  case kEnv3Int:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[1].m_pGain.set(127*GetParam(kEnv3Int)->Value());
    break;
  case kEnv3Atk:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[1].setPeriod(0, GetParam(kEnv3Atk)->Value(), 0);
    break;
  case kEnv3Dec:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[1].setPeriod(1, GetParam(kEnv3Dec)->Value(), 0);
    break;
  case kEnv3Rel:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[1].setPeriod(2, GetParam(kEnv3Rel)->Value(), 0);
    break;
  case kEnv3Sus:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[1].setPoint(1, GetParam(kEnv3Sus)->Value());
    break;
  case kEnv3Shp:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[1].setShape(-1, GetParam(kEnv3Shp)->Value());
    break;
    /* Env 4 */ 
  case kEnv4Int:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[2].m_pGain.set(127 * GetParam(kEnv4Int)->Value());
    break;
  case kEnv4Atk:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[2].setPeriod(0, GetParam(kEnv4Atk)->Value(), 0);
    break;
  case kEnv4Dec:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[2].setPeriod(1, GetParam(kEnv4Dec)->Value(), 0);
    break;
  case kEnv4Rel:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[2].setPeriod(2, GetParam(kEnv4Rel)->Value(), 0);
    break;
  case kEnv4Sus:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[2].setPoint(1, GetParam(kEnv4Sus)->Value());
    break;
  case kEnv4Shp:
    while (n--)
      mVoiceManager.m_voices[n].mVFEnv[2].setShape(-1, GetParam(kEnv4Shp)->Value());
    break;

    /* Osc 1 */
  case kOsc1Tune:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[0].m_pPitch.bias(1 * GetParam(kOsc1Tune)->Value());
    break;
  case kOsc1VF:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[0].mpPulsePitch.set(127 * GetParam(kOsc1VF)->Value());
    break;
  case kOsc1Num:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[0].mpNumber.set(GetParam(kOsc1Num)->Value());
    break;
  case kOsc1Dec:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[0].mpDecay.set(LERP(0, -12, GetParam(kOsc1Dec)->Value()));
    break;
  case kOsc1FreqMode:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[0].toggleRelativePFreq(GetParam(kOsc1FreqMode)->Value());
    break;
  case kOsc1Vol:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[0].m_pGain.set(dbToAmp(LERP(-60, 6, GetParam(kOsc1Vol)->Value())));
    break;

    /* Osc 2 */ 
  case kOsc2Tune:
      while (n--)
        mVoiceManager.m_voices[n].mOsc[1].m_pPitch.bias(1 * GetParam(kOsc2Tune)->Value());
      break;
  case kOsc2VF:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[1].mpPulsePitch.set(127 * GetParam(kOsc2VF)->Value());
    break;
  case kOsc2Num:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[1].mpNumber.set(GetParam(kOsc2Num)->Value());
    break;
  case kOsc2Dec:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[1].mpDecay.set(LERP(0, -12, GetParam(kOsc2Dec)->Value()));
    break;
  case kOsc2FreqMode:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[1].toggleRelativePFreq(GetParam(kOsc2FreqMode)->Value());
    break;
  case kOsc2Vol:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[1].m_pGain.set(dbToAmp(LERP(-60, 12, GetParam(kOsc2Vol)->Value())));
    break;

    /* Osc 3 */
  case kOsc3Tune:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[2].m_pPitch.bias(1 * GetParam(kOsc3Tune)->Value());
    break;
  case kOsc3VF:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[2].mpPulsePitch.set(127 * GetParam(kOsc3VF)->Value());
    break;
  case kOsc3Num:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[2].mpNumber.set(GetParam(kOsc3Num)->Value());
    break;
  case kOsc3Dec:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[2].mpDecay.set(LERP(0, -12, GetParam(kOsc3Dec)->Value()));
    break;
  case kOsc3FreqMode:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[2].toggleRelativePFreq(GetParam(kOsc3FreqMode)->Value());
    break;
  case kOsc3Vol:
    while (n--)
      mVoiceManager.m_voices[n].mOsc[2].m_pGain.set(dbToAmp(LERP(-60, 12, GetParam(kOsc3Vol)->Value())));
    break;

    /* Global Pitch LFO */
  case kGlobalPMF:
    while (n--)
      mVoiceManager.m_voices[n].mLFOPitch.m_pPitch.set(LERP(-64,64,GetParam(kGlobalPMF)->Value()));
    break;
  case kGlobalPMG:
    while (n--)
      // ampToDb(LERP(1/(2*nOctaves), 2*nOctaves, x))-ampToDb(1./(2*nOctaves))
      //mVoiceManager.m_voices[n].mLFOPitch.m_pGain.set(ampToDb(LERP(1./2, 2, GetParam(kGlobalPMG)->Value()))-ampToDb(1./2));
      mVoiceManager.m_voices[n].mLFOPitch.m_pGain.set(LERP(0,12, GetParam(kGlobalPMG)->Value()));
    break;
  default:
    break;
  }
}
