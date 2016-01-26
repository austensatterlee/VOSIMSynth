#include "VoiceManager.h"
namespace syn
{
  int VoiceManager::createVoice(int note, int vel)
  {
    int vind = findIdleVoice();
    m_idleVoiceStack.remove(vind);
    m_voiceStack.push_back(vind);
    m_voiceMap[note].push_back(vind);
    m_allVoices[vind]->noteOn(note, vel);
    m_numVoices++;
    return vind;
  }

  void VoiceManager::makeIdle()
  {
    if (m_numVoices > 0) {
      int vind = getOldestVoiceInd();
      makeIdle(vind);
    }
  }

  void VoiceManager::makeIdle(int vind)
  {
    if (m_numVoices > 0) {
      if(m_voiceMap.find(m_allVoices[vind]->getNote())!=m_voiceMap.end()){
        m_voiceMap[m_allVoices[vind]->getNote()].remove(vind);     
        if (m_voiceMap[m_allVoices[vind]->getNote()].empty())
        {
          m_voiceMap.erase(m_allVoices[vind]->getNote());
        }
        m_voiceStack.remove(vind);
        m_idleVoiceStack.push_front(vind);
        m_numVoices--;
      }
    }
  }

  int VoiceManager::findIdleVoice()
  {
    int vind = m_idleVoiceStack.front();
    m_idleVoiceStack.pop_front();
    return vind;
  }

  void VoiceManager::noteOn(uint8_t noteNumber, uint8_t velocity)
  {
    if (m_numVoices == m_maxVoices - 1)
    {
      Instrument* v;
      int vind = getOldestVoiceInd();
      m_idleVoiceStack.remove(vind);
      v = m_allVoices[vind];
      m_voiceMap[v->getNote()].remove(vind);
      m_voiceStack.remove(vind);
      v->noteOn(noteNumber, velocity);
      m_voiceStack.push_back(vind);
      m_voiceMap[noteNumber].push_back(vind);
    }
    else if (m_numVoices >= 0)
    {
      createVoice(noteNumber, velocity);
    }
  }

  void VoiceManager::noteOff(uint8_t noteNumber, uint8_t velocity)
  {
    if (m_voiceMap.find(noteNumber) != m_voiceMap.end() && !m_voiceMap[noteNumber].empty())
    {
      for (VoiceList::iterator v = m_voiceMap[noteNumber].begin(); v != m_voiceMap[noteNumber].end(); ++v)
      {
        m_allVoices[*v]->noteOff(noteNumber, velocity);
      }
    }
  }

  void VoiceManager::onHostReset(double fs, size_t bufsize, double tempo)
  {
    m_instrument->setSampleRate(fs);
	m_instrument->setBufferSize(bufsize);
	m_instrument->setTempo(tempo);
    for (vector<Instrument*>::iterator v = m_allVoices.begin(); v != m_allVoices.end(); ++v)
    {
	  (*v)->setSampleRate(fs);
	  (*v)->setBufferSize(bufsize);
	  (*v)->setTempo(tempo);
    }
  }

  void VoiceManager::setMaxVoices(int max, Instrument* v)
  {
    if (max < 1)
      max = 1;
    m_maxVoices = max;
    while (m_voiceStack.size() > 0)
    {
      makeIdle();
      m_idleVoiceStack.pop_front();
    }

    while (m_idleVoiceStack.size() > 0)
    {
      m_idleVoiceStack.pop_back();
    }

    while (m_allVoices.size() > max)
    {
      delete m_allVoices.back();
      m_allVoices.pop_back();
    }

    m_instrument = v;

    for (int i = 0; i < m_allVoices.size(); i++)
    {
      delete m_allVoices[i];
      m_allVoices[i] = static_cast<Instrument*>(m_instrument->clone());
      m_idleVoiceStack.push_back(i);
    }

    while (m_allVoices.size() < max)
    {
      m_allVoices.push_back(static_cast<Instrument*>(m_instrument->clone()));
      m_idleVoiceStack.push_back(m_allVoices.size()-1);
    }
    m_numVoices = 0;
  }

  void VoiceManager::tick(double** buf, size_t bufsize)
  {
    vector<int> garbage_list;
    for (VoiceList::const_iterator v = m_voiceStack.begin(); v != m_voiceStack.end(); v++)
    {
      Instrument* voice = m_allVoices[*v];
      if (voice->isActive())
      {
        voice->tick();
        const vector<UnitSample>& voicebuf = voice->getLastOutputBuffer();
        for (int i = 0; i < voice->getBufSize(); i++) {
			buf[0][i] += voicebuf[i][0];
			buf[1][i] += voicebuf[i][1];
        }
      }
      else
      {
        garbage_list.push_back(*v);
      }
    }
    for (int i = 0; i < garbage_list.size(); i++)
    {
      makeIdle(garbage_list[i]);
    }
  }

  void VoiceManager::modifyParameter(int uid, int pid, double val, MOD_ACTION action)
  {
    m_instrument->modifyParameter(uid, pid, val, action);
    for (int i = 0; i < m_allVoices.size(); i++)
    {
      m_allVoices[i]->modifyParameter(uid, pid, val, action);
    }
  }

  int VoiceManager::getLowestVoiceInd() const
  {
    if (m_numVoices > 0)
    {
      VoiceMap::const_iterator it;
      for (it = m_voiceMap.begin(); it != m_voiceMap.end(); it++)
      {
        if (!it->second.empty() && m_allVoices[it->second.back()]->isActive())
        {
          return it->second.back();
        }
      }
    }
    return 0;
  }

  int VoiceManager::getNewestVoiceInd() const
  {
    if (m_numVoices > 0)
    {
      for (VoiceList::const_reverse_iterator it = m_voiceStack.crbegin(); it != m_voiceStack.crend(); ++it)
      {
        if (m_allVoices[*it]->isActive())
          return *it;
      }
    }
    return 0;
  }

  int VoiceManager::getOldestVoiceInd() const
  {
    if (m_numVoices > 0)
    {
      for (VoiceList::const_iterator it = m_voiceStack.cbegin(); it != m_voiceStack.cend(); ++it)
      {
        if (m_allVoices[*it]->isActive())
          return *it;
      }
    }
    return 0;
  }

  int VoiceManager::getHighestVoiceInd() const
  {
    if (m_numVoices > 0)
    {
      VoiceMap::const_reverse_iterator it;
      for (it = m_voiceMap.crbegin(); it != m_voiceMap.crend(); ++it)
      {
        if (!it->second.empty() && m_allVoices[it->second.back()]->isActive())
        {
          return it->second.back();
        }
      }
    }
    return 0;
  }
}