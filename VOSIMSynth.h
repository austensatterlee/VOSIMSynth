#ifndef __VOSIMSYNTH__
#define __VOSIMSYNTH__

#include "IPlug_include_in_plug_hdr.h"
#include "resource.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"
#include "Instrument.h"
#include "Oscilloscope.h"
#include <vector>

using namespace syn;
using namespace std;

class VOSIMSynth : public IPlug {
public:
  VOSIMSynth(IPlugInstanceInfo instanceInfo);
  void makeGraphics();
  void makeInstrument();
  void unitTickHook(double input);
  void OnNoteOn(uint8_t pitch, uint8_t vel);
  void OnDyingVoice(Instrument* dying);
  ~VOSIMSynth()
  {
  };

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  void ProcessMidiMsg(IMidiMsg* pMsg);
  void SetInstrParameter(int unitid, int paramid, double value);
  double GetInstrParameter(int unitid, int paramid);
private:
  MIDIReceiver m_MIDIReceiver;
  VoiceManager m_voiceManager;
  Oscilloscope m_Oscilloscope;
  Instrument* m_instr;
  vector<pair<int, int>> m_hostParamMap;
  unordered_map<int,unordered_map<int,int>> m_invHostParamMap;
  IGraphics* pGraphics;
  int m_numParameters;
  unsigned int m_sampleCount;
};

#endif
