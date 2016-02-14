#ifndef __VOSIMSYNTH__
#define __VOSIMSYNTH__

#include "IPlug_include_in_plug_hdr.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"
#include "UnitFactory.h"
#include "CircuitPanel.h"
#include "IGraphics.h"

using namespace syn;
using namespace std;

class VOSIMSynth : public IPlug {
public:
    VOSIMSynth(IPlugInstanceInfo instanceInfo);

    void makeGraphics();

    void makeInstrument();

    virtual ~VOSIMSynth()
    { };

    virtual void Reset() override;

    void OnParamChange(int paramIdx) override;

    virtual void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) override;

    virtual void ProcessMidiMsg(IMidiMsg* pMsg) override;

    virtual bool SerializeState(ByteChunk* pChunk) override;

    virtual int UnserializeState(ByteChunk* pChunk, int startPos) override;

    virtual void PresetsChangedByHost() override;

private:
    shared_ptr<MIDIReceiver> m_MIDIReceiver;
	shared_ptr<VoiceManager> m_voiceManager;
	shared_ptr<CircuitPanel> m_circuitPanel;
	shared_ptr<UnitFactory> m_unitFactory;
    IGraphics* pGraphics;
};

#endif
