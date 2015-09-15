#ifndef __VOICEMANAGER__
#define __VOICEMANAGER__

#define MOD_FS_RAT 1
#include "Oscillator.h"
#include <cmath>
#include <cstdint>

inline double noteNumberToFrequency(int noteNumber) { return 440.0 * pow(2.0, (noteNumber - 69.0) / 12.0); }

class Voice {
private:
public:
	uint8_t mNote;
	double mVelocity;
	VOSIM mOsc[3];
    Envelope mVFEnv[3];
	Envelope mAmpEnv;
	Oscillator mLFOPitch;
	void trigger(uint8_t noteNumber, uint8_t velocity);
	void release();
	void reset();
	void setAudioFs(double fs);
	void setModFs(double fs);
	void tickAudio();
	void tickMod();
	double getOutput();
	bool isActive();
	Voice() {
		mNote = 0;
		mVelocity = 0;
		mLFOPitch.mWaveform = SINE_WAVE;
		mOsc[0].setGain(1);
		mOsc[1].setGain(1);
		mOsc[2].setGain(1);
	};
};

class VoiceManager {
private:
	uint32_t mSampleCount;
public:
	uint8_t mNumVoices;
	Voice *voices;
	void TriggerNote(uint8_t noteNumber, uint8_t velocity);
	void ReleaseNote(uint8_t noteNumber, uint8_t velocity);
	void setFs(double fs);
	void setNumVoices(uint8_t numVoices);
	double tick();
	VoiceManager() :
		mNumVoices(0),
		mSampleCount(0)
	{
	};
};

#endif
