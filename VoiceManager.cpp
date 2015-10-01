#include "VoiceManager.h"

void VoiceManager::TriggerNote(uint8_t noteNumber, uint8_t velocity) {
  uint8_t i = mNumVoices;
  while (i--) {
    if (!mVoices[i].isActive()) {
      mVoices[i].trigger(noteNumber, velocity);
      mVoiceStack.push_front(&mVoices[i]);
      break;
    }
  }
}

void VoiceManager::ReleaseNote(uint8_t noteNumber, uint8_t velocity) {
  uint8_t i = mNumVoices;
  while (i--) {
    if (mVoices[i].mNote == noteNumber) {
      mVoices[i].release();
    }
  }
}

void VoiceManager::setFs(double fs) {
  uint8_t i = mNumVoices;
  while (i--) {
    mVoices[i].setAudioFs(fs);
    mVoices[i].setModFs(fs / (MOD_FS_RAT + 1));
  }
}

void VoiceManager::setNumVoices(uint8_t numVoices) {
  mNumVoices = numVoices;
  mVoices.resize(mNumVoices);
}

double VoiceManager::tick() {
  double finalOutput = 0;
  mSampleCount++;
  for (list<Voice*>::iterator v=mVoiceStack.begin();v!=mVoiceStack.end();) {
    if ((*v)->isActive()) {
      if (!(mSampleCount & MOD_FS_RAT)) {
        (*v)->tickMod();
      }
      finalOutput += (*v)->process();
      v++;
    } else {
      (*v)->reset();
      v = mVoiceStack.erase(v++);
    }
  }
  finalOutput /= mNumVoices;
  return finalOutput;
}

void Voice::trigger(uint8_t noteNumber, uint8_t velocity) {
  mNote = noteNumber;
  mVelocity = velocity*0.0078125;
  mOsc[0].m_pPitch.set(mNote);mOsc[0].sync();
  mOsc[1].m_pPitch.set(mNote);mOsc[1].sync();
  mOsc[2].m_pPitch.set(mNote);mOsc[2].sync();
  mLFOPitch.sync();
  mAmpEnv.trigger();
  mVFEnv[0].trigger();
  mVFEnv[1].trigger();
  mVFEnv[2].trigger();
}

void Voice::release() {
  mAmpEnv.release();
  mVFEnv[0].release();
  mVFEnv[1].release();
  mVFEnv[2].release();
}

void Voice::reset() {
  mNote = 0;
  mVelocity = 0;
}

void Voice::setAudioFs(double fs) {
  mOsc[0].setFs(fs);
  mOsc[1].setFs(fs);
  mOsc[2].setFs(fs);
}

void Voice::setModFs(double fs) {
  mAmpEnv.setFs(fs);
  mLFOPitch.setFs(fs);
  mVFEnv[0].setFs(fs);
  mVFEnv[1].setFs(fs);
  mVFEnv[2].setFs(fs);
}

void Voice::tickMod() {
  mLFOPitch.updateParams();

  //mOsc[0].m_pPitch.mod(mLFOPitch.getLastOutput());
  //mOsc[0].mpPulsePitch.scale(mVFEnv[0].getLastOutput());
  mOsc[0].updateParams();

  //mOsc[1].m_pPitch.mod(mLFOPitch.getLastOutput());
  //mOsc[1].mpPulsePitch.scale(mVFEnv[1].getLastOutput());
  mOsc[1].updateParams();

  //mOsc[2].m_pPitch.mod(mLFOPitch.getLastOutput());
  //mOsc[2].mpPulsePitch.scale(mVFEnv[2].getLastOutput());
  mOsc[2].updateParams();
}

double Voice::process(double input) {
  double output, output1, output2, output3;
  output1 = mOsc[0].process(0);
  output2 = mOsc[1].process(0);
  output3 = mOsc[2].process(0);
  mAmpEnv.process(mVelocity);
  mLFOPitch.process(0);
  mVFEnv[0].process(0);
  mVFEnv[1].process(0);
  mVFEnv[2].process(0);
  output = (output1 + output2 + output3)*mVelocity;
  return output;
}

bool Voice::isActive() {
  return !mAmpEnv.isDone();
}
