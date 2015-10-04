#include "MIDIReceiver.h"

void MIDIReceiver::onMessageReceived(IMidiMsg* midiMessage) {
	IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
	// We're only interested in Note On/Off messages (not CC, pitch, etc.)
	mMidiQueue.Add(midiMessage);
}

void MIDIReceiver::advance() {
	while (!mMidiQueue.Empty()) {
		IMidiMsg* midiMessage = mMidiQueue.Peek();
		if (midiMessage->mOffset > mOffset) break;

		IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
		int noteNumber = midiMessage->NoteNumber();
		int velocity = midiMessage->Velocity();
		// There are only note on/off messages in the queue, see ::OnMessageReceived
		if (status == IMidiMsg::kNoteOn) {
			mKeyStatus[noteNumber] = true;
			mNumKeys += 1;
			noteOn(noteNumber, velocity);
		}
		else if(status == IMidiMsg::kNoteOff) {
			mKeyStatus[noteNumber] = false;
			mNumKeys -= 1;
			noteOff(noteNumber, velocity);
		}
		mMidiQueue.Remove(); 
	}
	mPrevNumKeys = mNumKeys;
	mOffset++;
}
