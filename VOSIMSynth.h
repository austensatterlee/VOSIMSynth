#ifndef __VOSIMSYNTH__
#define __VOSIMSYNTH__

#define NUM_VOICES 16

#include "IPlug_include_in_plug_hdr.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"
#include "Instrument.h"
#include "Connection.h"
#include "Filter.h"
#include "UI.h"
#include "Oscilloscope.h"
#include "EnvelopeEditor.h"
#include "IControl.h"
#include "resource.h"
#include "UI.h"
#include "Envelope.h"
#include "Oscillator.h"
#include "VosimOscillator.h"

using namespace syn;

class VOSIMSynth : public IPlug {
public:
  VOSIMSynth(IPlugInstanceInfo instanceInfo);
  void makeGraphics();
  void makeInstrument();
  void OnNoteOn(uint8_t pitch, uint8_t vel);
  void OnNoteOff(uint8_t pitch, uint8_t vel);
  void OnDyingVoice(Instrument* dying);
  ~VOSIMSynth()
  {
  };

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  void ProcessMidiMsg(IMidiMsg* pMsg);
private:
  MIDIReceiver m_MIDIReceiver;
  VoiceManager m_voiceManager;
  Oscilloscope m_Oscilloscope;
  Instrument* m_instr;
  vector<pair<int,int>> m_hostParamMap;
  int m_numParameters;
  double mLastOutput = 0.0;
};

#endif
