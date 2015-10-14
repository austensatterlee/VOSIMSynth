#ifndef __VOICEMANAGER__
#define __VOICEMANAGER__

#define MOD_FS_RAT 0
#include "Instrument.h"
#include <stdint.h>
#include <string>
#include <list>
#include <map>
using std::list;
using std::map;
using std::string;
namespace syn
{
  class VoiceManager
  {
  protected:
    typedef list<Instrument*> VoiceList;
    typedef map<int, VoiceList> VoiceMap;
    uint64_t mSampleCount;
    int m_numVoices;
    int m_maxVoices;
    VoiceMap m_voiceMap;
    VoiceList m_voiceStack;
    Instrument* m_instrument;
    Instrument* createVoice(int note,int vel);
    void deleteVoice();
    void deleteVoice(Instrument* v);

  public:
    Instrument* noteOn(uint8_t noteNumber, uint8_t velocity);
    void noteOff(uint8_t noteNumber, uint8_t velocity);
    Instrument* getLowestVoice() const;
    Instrument* getNewestVoice() const;
    Instrument* getOldestVoice() const;
    Instrument* getHighestVoice() const;
    void setFs(double fs);
    int getNumVoices() const { return m_numVoices; };
    int getMaxVoices(){return m_maxVoices;};
    void modifyParameter(string uname, string pname, MOD_ACTION action, double val);
    void setInstrument(Instrument* v);
    void setMaxVoices(int max);
    double tick();

    VoiceManager() :
      m_numVoices(0),
      mSampleCount(0)
    {
    };
    ~VoiceManager() {}
  };
}
#endif
