#ifndef __MIDIRECEIVER__
#define __MIDIRECEIVER__
#include <cstdint>
#include "IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"
#include "GallantSignal.h"
using Gallant::Signal2;
using Gallant::Signal1;

/**
 * \brief Midi queue handler 
 * \todo Integrate this into Circuit so messages can be communicated to Units mid-buffer
 */
class MIDIReceiver {
private:
	IMidiQueue mMidiQueue;
	static const int keyCount = 128;
	int mNumKeys, mPrevNumKeys; // how many keys are being played at the moment (via midi)
	bool mKeyStatus[keyCount]; // array of on/off for each key (index is note number)
	int mOffset;

public:
	Signal2<uint8_t, uint8_t> noteOn;
	Signal2<uint8_t, uint8_t> noteOff;
  Signal1<IMidiMsg&> sendControlChange;
	MIDIReceiver() :
		mNumKeys(0),
		mOffset(0) {
		for (int i = 0; i < keyCount; i++) {
			mKeyStatus[i] = false;
		}
	};

	// Returns true if the key with a given index is currently pressed
	inline bool getKeyStatus(int keyIndex) const { return mKeyStatus[keyIndex]; }
	// Returns the number of keys currently pressed
	inline int getNumKeys() const { return mNumKeys; }
	void advance();
	void onMessageReceived(IMidiMsg* midiMessage);
	inline void Flush(int nFrames) { mMidiQueue.Flush(nFrames); mOffset = 0; }
	inline void Resize(int blockSize) { mMidiQueue.Resize(blockSize); }
};
#endif