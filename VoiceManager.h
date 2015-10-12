
/*
#ifndef __VOICEMANAGER__
#define __VOICEMANAGER__

#define MOD_FS_RAT 0
#include "Oscillator.h"
#include "Filter.h"
#include "Unit.h"
#include <cmath>
#include <cstdint>
#include <string>
#include <list>
#include <vector>
#include <map>

struct ModMessageTemplate
{
  string m_cname;
  string m_pname;
  MOD_ACTION m_action;
  ModMessageTemplate(string cname, string pname, MOD_ACTION action)
  {
    m_cname = cname;
    m_pname = pname;
    m_action = action;
  }
};

class Voice : public Unit
{
public:
  uint8_t mNote;
  double mVelocity;
  void add_component(string name, Unit& component);
  void add_connection(string cname_from, string cname_to, string target_pname, MOD_ACTION action);
  void modifyParameter(string cname, string pname, MOD_ACTION action, double value);
  void modifyParameter(string pname, MOD_ACTION action, double value);
  void trigger(uint8_t noteNumber, uint8_t velocity);
  void release();
  void setFs(double fs);
  bool isActive();
  bool isSynced();
  double process(const double input = 0);
  int getSamplesPerPeriod() const;
  Voice() :
    Unit()
  {
    mNote = 0;
    mVelocity = 0;
  };
  Voice(const Voice& v) :
    Unit(v)
  {
    mNote = v.mNote;
    mVelocity = v.mVelocity;
    std::copy(v.m_params.begin(), v.m_params.end(), m_params);
    for (auto it = v.m_components.begin(); it != v.m_components.end(); it++)
    {
      m_components[it] =
    }
  }
  ~Voice()
  {}
protected:
  map<string, Unit*> m_components;
  map<string, list<ModMessageTemplate>> m_connections;
  bool m_isActive = false;
};


class VoiceManager
{
private:
  uint32_t mSampleCount;
  uint8_t m_numVoices;
public:
  map<int, list<Voice*> > m_voiceMap;
  list<Voice*> m_activeVoiceStack;
  list<Voice*> m_idleVoiceStack;
  vector<Voice> m_voices;
  Voice& TriggerNote(uint8_t noteNumber, uint8_t velocity);
  void ReleaseNote(uint8_t noteNumber, uint8_t velocity);
  Voice& getLowestVoice() const;
  Voice& getNewestVoice() const;
  Voice& getOldestVoice() const;
  Voice& getHighestVoice() const;
  void setFs(double fs);
  int getNumVoices() const { return m_numVoices; };
  int getNumActiveVoices() const { return m_activeVoiceStack.size(); };
  double process(double input = 0);
  void modifyParameter(string cname, string pname, MOD_ACTION action, double val);
  void modifyParameter(string pname, MOD_ACTION action, double val);
  void setVoice(Voice& v, int numVoices);

  VoiceManager() :
    m_numVoices(1),
    mSampleCount(0)
  {
    setVoice(Voice(), m_numVoices);
  };
  ~VoiceManager() {}
};

#endif
*/
