#include "VoiceManager.h"
namespace syn
{
  Instrument* VoiceManager::createVoice(int note, int vel)
  {
    m_voiceStack.push_back((Instrument*)m_instrument->clone());
    m_voiceMap[note].push_back(m_voiceStack.back());
    m_voiceStack.back()->noteOn(note, vel);
    m_numVoices++;
    return m_voiceStack.back();
  }

  void VoiceManager::deleteVoice()
  {
    Instrument* v = m_voiceMap.begin()->second.back();
    m_voiceMap.begin()->second.pop_back();
    m_voiceStack.remove(v);
    m_numVoices--;
    m_onDyingVoice.Emit(v);
    delete v;
  }

  void VoiceManager::deleteVoice(Instrument* v)
  {
    m_voiceMap[v->getNote()].remove(v);
    m_voiceStack.remove(v);
    m_numVoices--;
    m_onDyingVoice.Emit(v);
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
      v = createVoice(noteNumber, velocity);
    }
    return v;
  }

  Instrument* VoiceManager::noteOff(uint8_t noteNumber, uint8_t velocity)
  {
    Instrument* v;
    if (m_voiceMap.find(noteNumber) != m_voiceMap.end() && !m_voiceMap[noteNumber].empty())
    {
      v = m_voiceMap[noteNumber].back();
      v->noteOff(noteNumber, velocity);
    }
    return v;
  }

  void VoiceManager::setFs(double fs)
  {
    m_instrument->setFs(fs);
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

  void VoiceManager::modifyParameter(int uid, int pid, double val, MOD_ACTION action)
  {
    m_instrument->modifyParameter(uid, pid, val, action);
    for (VoiceList::iterator v = m_voiceStack.begin(); v != m_voiceStack.end(); v++)
    {
      (*v)->modifyParameter(uid, pid, val, action);
    }
  }

  void VoiceManager::sendMIDICC(IMidiMsg& msg)
  {
    m_instrument->sendMIDICC(msg);
    for (VoiceList::iterator v = m_voiceStack.begin(); v != m_voiceStack.end(); v++)
    {
      (*v)->sendMIDICC(msg);
    }
  }

  Instrument* VoiceManager::getLowestVoice() const
  {

    if (m_numVoices)
    {
      VoiceMap::const_iterator it;
      for (it = m_voiceMap.begin(); it != m_voiceMap.end(); it++)
      {
        if (!it->second.empty() && it->second.back()->isActive())
        {
          return it->second.back();
        }
      }
    }
    return nullptr;
  }

  Instrument* VoiceManager::getNewestVoice() const
  {
    if (m_numVoices){
      for (VoiceList::const_reverse_iterator it = m_voiceStack.rbegin(); it!=m_voiceStack.rend();it++)
      {
        if((*it)->isActive())
          return *it;
      }
    }
    return nullptr;
  }

  Instrument* VoiceManager::getOldestVoice() const
  {
    if (m_numVoices)
    {
      for (VoiceList::const_iterator it = m_voiceStack.begin(); it != m_voiceStack.end(); it++)
      {
        if ((*it)->isActive())
          return *it;
      }
    }
    return nullptr;
  }

  Instrument* VoiceManager::getHighestVoice() const
  {
    if (m_numVoices)
    {
      VoiceMap::const_reverse_iterator it;
      for (it = m_voiceMap.rbegin(); it != m_voiceMap.rend(); it++)
      {
        if (!it->second.empty() && it->second.back()->isActive())
        {
          return it->second.back();
        }
      }
    }
    return nullptr;
  }
}