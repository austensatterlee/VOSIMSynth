#include "VoiceManager.h"

Voice& VoiceManager::TriggerNote(uint8_t noteNumber, uint8_t velocity) {
  Voice* v;
  if (m_idleVoiceStack.empty()) {
    v = m_activeVoiceStack.back();
    m_activeVoiceStack.pop_back();
    m_idleVoiceStack.push_front(v);
    ReleaseNote(noteNumber, velocity);
  }
  v = m_idleVoiceStack.front();
  m_idleVoiceStack.pop_front();
  v->trigger(noteNumber, velocity);
  m_activeVoiceStack.push_front(v);
  m_voiceMap[noteNumber].push_front(v);
  return *v;
}

void VoiceManager::ReleaseNote(uint8_t noteNumber, uint8_t velocity) {
  if (!m_voiceMap[noteNumber].empty()) {
    for (list<Voice*>::iterator v = m_voiceMap[noteNumber].begin(); v != m_voiceMap[noteNumber].end();) {
      (*v)->release();
      v = m_voiceMap[noteNumber].erase(v++);
    }
    m_voiceMap.erase(noteNumber);
  }
}

void VoiceManager::setFs(double fs) {
  uint8_t i = m_numVoices;
  while (i--) {
    m_voices[i].setAudioFs(fs);
    m_voices[i].setModFs(fs / (MOD_FS_RAT + 1));
  }
}

void VoiceManager::setNumVoices(int numVoices) {
  if (numVoices<1)
    numVoices = 1;
  m_numVoices = numVoices;
  m_voices.resize(m_numVoices);
  m_activeVoiceStack.clear();
  m_idleVoiceStack.clear();
  m_voiceMap.clear();
  for (vector<Voice>::iterator v = m_voices.begin(); v != m_voices.end(); v++) {
    m_idleVoiceStack.push_front(&(*v));
  }
}

double VoiceManager::process(double input) {
  double finalOutput = 0;
  mSampleCount++;
  for (list<Voice*>::iterator v = m_activeVoiceStack.begin(); v != m_activeVoiceStack.end();) {
    if ((*v)->isActive()) {
      if (!(mSampleCount & MOD_FS_RAT)) {
        (*v)->updateParams();
      }
      finalOutput += (*v)->process();
      v++;
    }
    else {
      m_idleVoiceStack.push_front(*v);
      v = m_activeVoiceStack.erase(v++);
    }
  }
  return finalOutput;
}

Voice& VoiceManager::getLowestVoice() const {
  /*map<int, list<Voice*>>::const_iterator v;
  for (v = m_voiceMap.begin(); v != m_voiceMap.end(); v++) {
    if (!(*v).second.empty())
      break;
  }
  return *(*v).second.front();*/
  return *(*m_voiceMap.begin()).second.front();
}

Voice & VoiceManager::getNewestVoice() const {
  return *m_activeVoiceStack.front();
}

Voice & VoiceManager::getOldestVoice() const {
  return *m_activeVoiceStack.back();
}

Voice & VoiceManager::getHighestVoice() const {
  return *(*m_voiceMap.end()).second.front();
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

bool Voice::isSynced() {
  return mOsc[0].isSynced() && mOsc[1].isSynced() && mOsc[2].isSynced();
}

void Voice::release() {
  mAmpEnv.release();
  mVFEnv[0].release();
  mVFEnv[1].release();
  mVFEnv[2].release();
}

void Voice::setAudioFs(double fs) {
  setFs(fs);
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

void Voice::updateParams() {
  DSPComponent::updateParams();
  mVFEnv[0].process(0);
  mVFEnv[1].process(0);
  mVFEnv[2].process(0);
  mLFOPitch.process(0);
  mAmpEnv.process(mVelocity);
  mVFEnv[0].updateParams();
  mVFEnv[1].updateParams();
  mVFEnv[2].updateParams();
  mLFOPitch.updateParams();
  mAmpEnv.updateParams();
  mOsc[0].updateParams();
  mOsc[1].updateParams();
  mOsc[2].updateParams();
}

double Voice::process(double input) {
  double output, output1, output2, output3;
  output1 = mOsc[0].process(0);
  output2 = mOsc[1].process(0);
  output3 = mOsc[2].process(0);
  output = (output1 + output2 + output3)*mVelocity;
  if(isSynced())
    triggerOut();
  return finishProcessing(output);
}

int Voice::getSamplesPerPeriod() const {
  int max_period = fmax(mOsc[0].getSamplesPerPeriod(),fmax(mOsc[1].getSamplesPerPeriod(),mOsc[2].getSamplesPerPeriod()));
  return max_period;
}

bool Voice::isActive() {
  return !mAmpEnv.isDone();
}
