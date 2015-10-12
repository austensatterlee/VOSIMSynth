#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include "UI.h"
#include "Envelope.h"
#include "Oscillator.h"
#include "VosimOscillator.h"
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

  TypedParamProp(const char *name, const T defVal, Parameter& parent, const T(*modify)(T, T)) :
    ParamProp(name),
    m_default(defVal),
    m_parent(parent),
    m_modify(modify) {
    if (is_integral<T>::value)
      m_step = 1;
    else
      m_step = 1E-3;
  };
private:
  const T m_default;
  const T m_step;
  Parameter& m_parent;
  const T(*m_modify)(T, T);
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
    
  m_MIDIReceiver.noteOn.Connect(this, &VOSIMSynth::OnNoteOn);
  m_MIDIReceiver.noteOff.Connect(this, &VOSIMSynth::OnNoteOff);

  m_circuit.setSink("sum", new AccumulatingSink());
  m_circuit.setSource("vosc", new VosimOscillator());
  m_circuit.addConnection(new Connection("vosc","sum","output",ADD));
}

void VOSIMSynth::OnNoteOn(uint8_t pitch, uint8_t vel) {
  IMutexLock lock(this);
  m_circuit.trigger();
  m_circuit.modUnitParam("vosc","pitch",SET,pitch);
}

void VOSIMSynth::OnNoteOff(uint8_t pitch, uint8_t vel) {
  IMutexLock lock(this);
  m_circuit.release();
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
  // Mutex is already locked for us.
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];
  for (int s = 0; s < nFrames; s++) {
    leftOutput[s] = 0;
    m_MIDIReceiver.advance();
    leftOutput[s] = mOutGain*m_circuit.tick();
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
  m_circuit.setFs(fs);
}

void VOSIMSynth::OnParamChange(int paramIdx) {
  IMutexLock lock(this);
  int n = 1;
  switch (paramIdx) {
  case kMainVol:
    mOutGain = dbToAmp(LERP(ampToDb(1./n), 0, GetParam(kMainVol)->Value()));
    break;
  default:
    break;
  }
}
