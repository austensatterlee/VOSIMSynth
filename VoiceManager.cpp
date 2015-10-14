#include "VoiceManager.h"
namespace syn
{
  Instrument* VoiceManager::createVoice(int note,int vel)
  {
    m_voiceStack.push_back(m_instrument->clone());
    m_voiceMap[note].push_back(m_voiceStack.back());
    m_voiceStack.back()->noteOn(note,vel);
    m_numVoices++;
    return m_voiceStack.back();
  }

  void VoiceManager::deleteVoice()
  {
    Instrument* v = m_voiceMap.begin()->second.back();
    m_voiceMap.begin()->second.pop_back();
    m_voiceStack.remove(v);
    m_numVoices--;
    delete v;
  }

  void VoiceManager::deleteVoice(Instrument* v)
  {
    m_voiceMap[v->getNote()].remove(v);
    m_voiceStack.remove(v);
    m_numVoices--;
    delete v;
  }

  Instrument* VoiceManager::noteOn(uint8_t noteNumber, uint8_t velocity)
  {
    Instrument* v;
    if (m_numVoices == m_maxVoices)
    {
      v = getLowestVoice();
      m_voiceMap[v->getNote()].remove(v);
      m_voiceStack.remove(v);
      v->noteOn(noteNumber, velocity);
      m_voiceStack.push_back(v);
      m_voiceMap[noteNumber].push_back(v);
    }
    else
    {
      v = createVoice(noteNumber,velocity);
    }
    return v;
  }

  void VoiceManager::noteOff(uint8_t noteNumber, uint8_t velocity)
  {
    if (m_voiceMap.find(noteNumber)!=m_voiceMap.end() && !m_voiceMap[noteNumber].empty())
    {
      for (VoiceList::iterator v = m_voiceMap[noteNumber].begin(); v != m_voiceMap[noteNumber].end();v++)
      {
        (*v)->noteOff(noteNumber, velocity);
      }
    }
  }

  void VoiceManager::setFs(double fs)
  {
    for (VoiceList::iterator v = m_voiceStack.begin(); v != m_voiceStack.end(); v++)
    {
      (*v)->setFs(fs);
    }
  }

  void VoiceManager::setInstrument(Instrument* v)
  {
    while (m_numVoices > 1)
    {
      deleteVoice();
    }
    m_instrument = v;
  }

  void VoiceManager::setMaxVoices(int max)
  {
    m_maxVoices = max;
    while (m_numVoices > max)
    {
      deleteVoice();
    }
  }

  double VoiceManager::tick()
  {
    double finalOutput = 0;
    mSampleCount++;

    for (VoiceList::iterator v = m_voiceStack.begin(); v != m_voiceStack.end();)
    {
      if ((*v)->isActive())
      {
        finalOutput += (*v)->tick();
        v++;
      }
      else
      {
        deleteVoice(*(v++));
      }
    }
    return finalOutput;
  }

  void VoiceManager::modifyParameter(string uname, string pname, MOD_ACTION action, double val)
  {
    for (VoiceList::iterator v = m_voiceStack.begin(); v != m_voiceStack.end(); v++)
    {
      (*v)->modifyParameter(uname, pname, action, val);
    }
  }

  Instrument* VoiceManager::getLowestVoice() const
{

    if (m_numVoices)
    {
      VoiceMap::const_iterator it;
      for( it = m_voiceMap.begin(); it->second.empty(); it++);
      return it->second.back();
    }
    else
    {
      return m_instrument;
    }
  }

  Instrument* VoiceManager::getNewestVoice() const
{
    if (m_numVoices)
      return m_voiceStack.front();
    else
      return m_instrument;
  }

  Instrument* VoiceManager::getOldestVoice() const
{
    if (m_numVoices)
      return m_voiceStack.back();
    else
      return m_instrument;
  }

  Instrument* VoiceManager::getHighestVoice() const
{
  if (m_numVoices)
  {
    VoiceMap::const_iterator it;
    for (it = m_voiceMap.end(); it->second.empty(); it--);
    return it->second.back();
  }
    else
      return m_instrument;
  }
}