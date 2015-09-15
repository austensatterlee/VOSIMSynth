#ifndef __VOSIMSYNTH__
#define __VOSIMSYNTH__

#define NUM_VOICES 8

#include <cstdint>
#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"
#include "Filter.h"

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
	Filter *LP4;
	double mVolume;
	int mOversampling;
};

#endif
