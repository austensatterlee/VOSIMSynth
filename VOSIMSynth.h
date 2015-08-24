#ifndef __VOSIMSYNTH__
#define __VOSIMSYNTH__

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"
#include <cstdint>
#define NUM_VOICES 24
#define MOD_FS_RAT 63

class VOSIMSynth : public IPlug
{
public:
	VOSIMSynth(IPlugInstanceInfo instanceInfo);
	~VOSIMSynth();

	void Reset();
	void OnParamChange(int paramIdx);
	void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
	void ProcessMidiMsg(IMidiMsg* pMsg);
	void TriggerNote(const int noteNumber, const int velocity);
	void ReleaseNote(const int noteNumber, const int velocity);
private:
	MIDIReceiver mMIDIReceiver;
	Envelope *mAmpEnv;
	VOSIM *mOsc;
	Oscillator mLFOPitch, mLFOVWidth;
	uint8_t mVoiceStack[NUM_VOICES][2] = { 0 };
	uint32_t mSampleCount;
};

#endif
