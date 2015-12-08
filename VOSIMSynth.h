#ifndef __VOSIMSYNTH__
#define __VOSIMSYNTH__

#include "IPlug_include_in_plug_hdr.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"
#include "Instrument.h"
#include "Oscilloscope.h"
#include "UnitFactory.h"
#include "CircuitPanel.h"
#include <vector>

using namespace syn;
using namespace std;

class VOSIMSynth : public IPlug {
public:
  VOSIMSynth(IPlugInstanceInfo instanceInfo);
  void makeGraphics();
  void makeInstrument();
  ~VOSIMSynth()
  {
  };

  virtual void Reset() override;
  void OnParamChange(int paramIdx) override;
  virtual void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) override;
  virtual void ProcessMidiMsg(IMidiMsg* pMsg) override;
  void SetInstrParameter(int unitid, int paramid, double value);
  double GetInstrParameter(int unitid, int paramid);

  virtual bool SerializeState(ByteChunk* pChunk) override;

  virtual int UnserializeState(ByteChunk* pChunk, int startPos) override;

  virtual void PresetsChangedByHost() override;
  Oscilloscope* m_Oscilloscope;
private:
  MIDIReceiver m_MIDIReceiver;
  VoiceManager m_voiceManager;
  CircuitPanel* m_circuitPanel;
  Instrument* m_instr;
  UnitFactory* m_unitfactory;

  vector<pair<int, int>> m_hostParamMap;
  unordered_map<int, unordered_map<int, int>> m_invHostParamMap;
  IGraphics* pGraphics;
  int m_numParameters;
  unsigned int m_sampleCount;
};

#endif
