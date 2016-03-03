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

    void Reset() override;

    void OnParamChange(int paramIdx) override;

    void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) override;

    void ProcessMidiMsg(IMidiMsg* pMsg) override;

    bool SerializeState(ByteChunk* pChunk) override;

    int UnserializeState(ByteChunk* pChunk, int startPos) override;

    void PresetsChangedByHost() override;

	void OnIdle() override;

	void OnActivate(bool active) override;

	bool isTransportRunning();

	int getTickCount() const { return m_tickCount; }

private:
    shared_ptr<MIDIReceiver> m_MIDIReceiver;
	shared_ptr<VoiceManager> m_voiceManager;
	shared_ptr<CircuitPanel> m_circuitPanel;
	shared_ptr<UnitFactory> m_unitFactory;
    IGraphics* pGraphics;

	int m_tempo;
	unsigned m_tickCount;
	ITimeInfo m_timeInfo;
};

#endif
