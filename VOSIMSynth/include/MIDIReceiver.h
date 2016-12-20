#ifndef __MIDIRECEIVER__
#define __MIDIRECEIVER__

#include "VOSIMSynth.h"
#include "IMidiQueue.h"

struct IMidiMsg;


namespace syn {

	class VoiceManager;

	/**
	 * \brief Midi queue handler
	 */
	class MIDIReceiver
	{
	public:
		explicit MIDIReceiver(VoiceManager *a_vm) :
			m_offset(0),
			m_vm(a_vm) {
			for (int i = 0; i < s_keyCount; i++) {
				m_keyStatus[i] = false;
			}
		};

		// Returns true if the key with a given index is currently pressed
		bool getKeyStatus(int keyIndex) const {
			return m_keyStatus[keyIndex];
		}

		void advance();

		void onMessageReceived(IMidiMsg *midiMessage);

		void Flush(int nFrames);

		void Resize(int blockSize);

	private:
		IMidiQueue m_midiQueue;
		static const int s_keyCount = 128;
		bool m_keyStatus[s_keyCount]; // array of on/off for each key (index is note number)
		int m_offset;
		VoiceManager *m_vm;
	};
}

#endif
