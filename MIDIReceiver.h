#ifndef __MIDIRECEIVER__
#define __MIDIRECEIVER__
#include "IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"
#include "GallantSignal.h"
using Gallant::Signal2;

inline double noteNumberToFrequency(int noteNumber) { return 440.0 * pow(2.0, (noteNumber - 69.0) / 12.0); }

class MIDIReceiver {
private:
	IMidiQueue mMidiQueue;
	static const int keyCount = 128;
	int mNumKeys, mPrevNumKeys; // how many keys are being played at the moment (via midi)
	bool mKeyStatus[keyCount]; // array of on/off for each key (index is note number)
	int mOffset;

public:
	Signal2<int, int> noteOn;
	Signal2<int, int> noteOff;
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