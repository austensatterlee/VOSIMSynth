#ifndef __VOICEMANAGER__
#define __VOICEMANAGER__

#define MOD_FS_RAT 1
#include "Oscillator.h"
#include <cmath>
#include <cstdint>
#include <list>
#include <vector>
#include <map>

class Voice : public DSPComponent<double> {
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
  void updateParams();
  bool isActive();
  bool isSynced();
  double process(double input = 0);
  int getSamplesPerPeriod() const;
  Voice() {
    mNote = 0;
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

class VoiceManager : public DSPComponent<double> {
private:
  uint32_t mSampleCount;
  uint8_t m_numVoices;
public:
  map<int,list<Voice*>> m_voiceMap;
  list<Voice*> m_activeVoiceStack;
  list<Voice*> m_idleVoiceStack;
  vector<Voice> m_voices;
  Voice& TriggerNote(uint8_t noteNumber, uint8_t velocity);
  Voice& ReleaseNote(uint8_t noteNumber, uint8_t velocity);
  void setFs(double fs);
  void setNumVoices(int numVoices);
  int getNumVoices() const {return m_numVoices;};
  double process(double input=0);
  Voice* getLowestVoice() const;
  int getSamplesPerPeriod() const;
  
  VoiceManager() :
    m_numVoices(1),
    mSampleCount(0) {      
      setNumVoices(m_numVoices);
    };
  ~VoiceManager() 
  {
  }
};

#endif
