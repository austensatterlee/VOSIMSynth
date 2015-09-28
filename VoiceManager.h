#ifndef __VOICEMANAGER__
#define __VOICEMANAGER__

#define MOD_FS_RAT 1
#include "Oscillator.h"
#include <cmath>
#include <cstdint>
#include <list>

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
    mOsc[0].mpGain.set(0);
    mOsc[1].mpGain.set(0);
    mOsc[2].mpGain.set(0);
  };
};

class VoiceManager {
private:
  uint32_t mSampleCount;
public:
  list<Voice*> mVoiceStack = list<Voice*>();
  uint8_t mNumVoices;
  Voice *mVoices;
  void TriggerNote(uint8_t noteNumber, uint8_t velocity);
  void ReleaseNote(uint8_t noteNumber, uint8_t velocity);
  void setFs(double fs);
  void setNumVoices(uint8_t numVoices);
  double tick();
  VoiceManager() :
    mNumVoices(1),
    mSampleCount(0) {};
};

#endif
