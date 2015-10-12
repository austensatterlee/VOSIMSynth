/*
#include "VoiceManager.h"

Voice& VoiceManager::TriggerNote(uint8_t noteNumber, uint8_t velocity) {
  Voice* v;
  if (m_idleVoiceStack.empty()) {
    v = &getLowestVoice();
    m_activeVoiceStack.remove(v);
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
    m_voices[i].setFs(fs)
  }
}

void VoiceManager::setVoice(Voice& v, int numVoices) {
  if (numVoices<1)
    numVoices = 1;
  m_numVoices = numVoices;
  m_voices.resize(m_numVoices);
  std:fill(m_voices.begin(),m_voices.end(),v);
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

void VoiceManager::modifyParameter(string cname, string pname, MOD_ACTION action, double val)
{
  for (vector<Voice>::iterator v = m_voices.begin(); v != m_voices.end(); v++)
  {
    (*v).modifyParameter(cname,pname,action,val);
  }
}

void VoiceManager::modifyParameter(string pname, MOD_ACTION action, double val)
{

  for (vector<Voice>::iterator v = m_voices.begin(); v != m_voices.end(); v++)
  {
    (*v).modifyParameter(pname, action, val);
  }
}

Voice& VoiceManager::getLowestVoice() const
{
 
  if(m_voiceMap.size()){
    return *(*m_voiceMap.begin()).second.front();
  }
  else if(m_activeVoiceStack.size()){
    int minnote = -1;
    Voice* minvoice;
    for (auto it = m_activeVoiceStack.begin(); it != m_activeVoiceStack.end(); it++) {
      if (minnote < (*it)->mNote || minnote == -1) {
        minvoice = *it;
        minnote = minvoice->mNote;
      }
    }
    return *minvoice;
  }
}

Voice& VoiceManager::getNewestVoice() const {
  return *m_activeVoiceStack.front();
}

Voice& VoiceManager::getOldestVoice() const {
  return *m_activeVoiceStack.back();
}

Voice& VoiceManager::getHighestVoice() const {
  return *(*m_voiceMap.end()).second.front();
}

void Voice::add_component(string name, Unit& component)
{
  m_components[name] = component;
}

void Voice::add_connection(string cname_from, string cname_to, string target_pname, MOD_ACTION action)
{
  m_components[cname_from].connectOutputTo(&m_components[cname_to], target_pname, action);
  m_connections[cname_to].push_back({cname_from,target_pname,action});
}

void Voice::modifyParameter(string pname, MOD_ACTION action, double val)
{
  for (auto it = m_components.begin(); it != m_components.end(); it++)
  {
    if (m_components[it].m_params.find(pname) != m_components[it].m_params.end())
    {
      m_components[it].modifyParameter(pname, action, val);
    }
  }
}

void Voice::modifyParameter(string cname, string pname, MOD_ACTION action, double val)
{
  m_components[cname].modifyParameter(pname, action, val);
}

void Voice::trigger(uint8_t noteNumber, uint8_t velocity)
{
  mNote = noteNumber;
  mVelocity = velocity*0.0078125;
  modifyParameter("pitch",SET,mNote);
  // todo
}

bool Voice::isSynced() {
  bool res = true;
  for (auto it = m_components.begin(); it != m_components.end(); it++)
  {
    if (m_components[it].m_params.find("pitch") != m_components[it].m_params.end())
    {
      res &= m_components[it].isSynced();
    }
  }
}

void Voice::release() {
  for (auto it = m_components.begin(); it != m_components.end(); it++)
  {
    m_components[it].release();
  }
}

void Voice::setFs(double fs) {
  for (auto it = m_components.begin(); it != m_components.end(); it++)
  {
    m_components[it].setFs(fs);
  }
}

double Voice::process(double input) {
  for (auto it = m_components.begin(); it != m_components.end(); it++)
  {
    m_components[it].process();
  }
  if(isSynced())
    syncOut();
  return finishProcessing();
}

int Voice::getSamplesPerPeriod() const {
  int max_period = 0;
  return max_period;
}

bool Voice::isActive() {
  return m_isActive;
}
*/
