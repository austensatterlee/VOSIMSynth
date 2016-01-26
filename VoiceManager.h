#ifndef __VOICEMANAGER__
#define __VOICEMANAGER__

#define MOD_FS_RAT 0
#include "Instrument.h"
#include <stdint.h>
#include <string>
#include <list>

using std::list;
using std::map;
using std::string;
namespace syn
{
	class VoiceManager
	{
	protected:
		typedef list<int> VoiceList;
		typedef map<int, VoiceList> VoiceMap;
		int m_numVoices;
		int m_maxVoices;
		VoiceMap m_voiceMap;
		VoiceList m_voiceStack;
		VoiceList m_idleVoiceStack;
		vector<Instrument*> m_allVoices;
		Instrument* m_instrument;
		int createVoice(int note, int vel);
		void makeIdle();
		int findIdleVoice();
		void makeIdle(int vind);

	public:
		void noteOn(uint8_t noteNumber, uint8_t velocity);
		void noteOff(uint8_t noteNumber, uint8_t velocity);
		Instrument* getLowestVoice() { int ind = getLowestVoiceInd(); return ind >= 0 ? m_allVoices[ind] : nullptr; };
		Instrument* getNewestVoice() { int ind = getNewestVoiceInd(); return ind >= 0 ? m_allVoices[ind] : nullptr; };
		Instrument* getOldestVoice() { int ind = getOldestVoiceInd(); return ind >= 0 ? m_allVoices[ind] : nullptr; };
		Instrument* getHighestVoice() { int ind = getHighestVoiceInd(); return ind >= 0 ? m_allVoices[ind] : nullptr; };
		int getLowestVoiceInd() const;
		int getNewestVoiceInd() const;
		int getOldestVoiceInd() const;
		int getHighestVoiceInd() const;

		Instrument* getProtoInstrument() const { return m_instrument; };
		void onHostReset(double fs, size_t bufsize, double tempo);
		void setMaxVoices(int max, Instrument* v);
		int getNumVoices() const { return m_numVoices; };
		int getMaxVoices() const { return m_maxVoices; };
		void modifyParameter(int uid, int pid, double val, MOD_ACTION action);
		void tick(double** buf, size_t bufsize);

		VoiceManager() :
			m_numVoices(0), m_maxVoices(0), m_instrument(nullptr)
		{
		};
		~VoiceManager() {}
	};
}
#endif
