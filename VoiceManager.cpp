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
  mVoices = new Voice[numVoices];
}

double VoiceManager::tick() {
  double finalOutput = 0;
  mSampleCount++;
  for (list<Voice*>::iterator v=mVoiceStack.begin();v!=mVoiceStack.end();) {
    if ((*v)->isActive()) {
      if (!(mSampleCount & MOD_FS_RAT)) {
        (*v)->tickMod();
      }
      (*v)->tickAudio();
      finalOutput += (*v)->getOutput();
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
  mOsc[0].mpPitch.set(mNote);mOsc[0].sync();
  mOsc[1].mpPitch.set(mNote);mOsc[1].sync();
  mOsc[2].mpPitch.set(mNote);mOsc[2].sync();
  mLFOPitch.sync();
  mAmpEnv.trigger();
  mVFEnv[0].trigger();
  mVFEnv[1].trigger();
  mVFEnv[2].trigger();
  tickMod();
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

void Voice::tickAudio() {
  mOsc[0].tick();
  mOsc[1].tick();
  mOsc[2].tick();
}

void Voice::tickMod() {
  mAmpEnv.tick();

  mVFEnv[0].tick();
  mVFEnv[1].tick();
  mVFEnv[2].tick();

  mLFOPitch.tick();
  mLFOPitch.update();

  mOsc[0].mpPitch.mod(mLFOPitch.getOutput());
  mOsc[0].mpPulsePitch.mod(LERP(0, 36, mVFEnv[0].getOutput()));
  mOsc[0].update();

  mOsc[1].mpPitch.mod(mLFOPitch.getOutput());
  mOsc[1].mpPulsePitch.mod(LERP(0, 36, mVFEnv[1].getOutput()));
  mOsc[1].update();

  mOsc[2].mpPitch.mod(mLFOPitch.getOutput());
  mOsc[2].mpPulsePitch.mod(LERP(0, 36, mVFEnv[2].getOutput()));
  mOsc[2].update();
}

double Voice::getOutput() {
  double output, output1, output2, output3;
  output1 = mOsc[0].getOutput();
  output2 = mOsc[1].getOutput();
  output3 = mOsc[2].getOutput();
  output = (output1 + output2 + output3)*mVelocity*mAmpEnv.getOutput()*0.3333333333333333;
  return output;
}

bool Voice::isActive() {
  return !mAmpEnv.mIsDone;
}
