#ifndef __VOICEMANAGER__
#define __VOICEMANAGER__

#define MOD_FS_RAT 1
#include "Oscillator.h"
#include <cmath>
#include <cstdint>
#include <list>
#include <vector>

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
  void tickMod();
  double process(double input=0);
  bool isActive();
  Voice() {
    mNote = 0;
    mVelocity = 0;
    mOsc[0].m_pGain.set(0);
    mOsc[1].m_pGain.set(0);
    mOsc[2].m_pGain.set(0);    
    mVFEnv[0].connectOutputTo(&mOsc[0].mpPulsePitch, &Modifiable<double>::mod);
    mVFEnv[1].connectOutputTo(&mOsc[1].mpPulsePitch, &Modifiable<double>::mod);
    mVFEnv[2].connectOutputTo(&mOsc[2].mpPulsePitch, &Modifiable<double>::mod);
    mAmpEnv.connectOutputTo(&mOsc[0].m_pGain, &Modifiable<double>::scale);
    mAmpEnv.connectOutputTo(&mOsc[1].m_pGain, &Modifiable<double>::scale);
    mAmpEnv.connectOutputTo(&mOsc[2].m_pGain, &Modifiable<double>::scale);
    mLFOPitch.connectOutputTo(&mOsc[0].m_pPitch, &Modifiable<double>::mod);
    mLFOPitch.connectOutputTo(&mOsc[1].m_pPitch, &Modifiable<double>::mod);
    mLFOPitch.connectOutputTo(&mOsc[2].m_pPitch, &Modifiable<double>::mod);
  };
};

class VoiceManager {
private:
  uint32_t mSampleCount;
public:
  list<Voice*> mVoiceStack;
  uint8_t mNumVoices;
  vector<Voice> mVoices;
  void TriggerNote(uint8_t noteNumber, uint8_t velocity);
  void ReleaseNote(uint8_t noteNumber, uint8_t velocity);
  void setFs(double fs);
  void setNumVoices(uint8_t numVoices);
  double tick();
  VoiceManager() :
    mNumVoices(1),
    mSampleCount(0) {
      mVoiceStack = list<Voice*>();
      mVoices = vector<Voice>(mNumVoices);
      setNumVoices(mNumVoices);
    };
  ~VoiceManager() 
  {
  }
};

#endif
