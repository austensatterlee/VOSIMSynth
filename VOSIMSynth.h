#ifndef __VOSIMSYNTH__
#define __VOSIMSYNTH__

#define NUM_VOICES 16

#include <cstdint>
#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"

class VOSIMSynth : public IPlug
{
public:
	VOSIMSynth(IPlugInstanceInfo instanceInfo);
	~VOSIMSynth();

	void Reset();
	void OnParamChange(int paramIdx);
	void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
	void ProcessMidiMsg(IMidiMsg* pMsg);
private:
	MIDIReceiver mMIDIReceiver;
	VoiceManager mVoiceManager;
	double mVolume;
};

#endif
