#include "MIDIReceiver.h"

void MIDIReceiver::onMessageReceived(IMidiMsg* midiMessage) {
	IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
	// We're only interested in Note On/Off messages (not CC, pitch, etc.)
	if (status == IMidiMsg::kNoteOn || status == IMidiMsg::kNoteOff) {
		mMidiQueue.Add(midiMessage);
	}
}

void MIDIReceiver::advance() {
	while (!mMidiQueue.Empty()) {
		IMidiMsg* midiMessage = mMidiQueue.Peek();
		if (midiMessage->mOffset > mOffset) break;

		IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
		int noteNumber = midiMessage->NoteNumber();
		int velocity = midiMessage->Velocity();
		// There are only note on/off messages in the queue, see ::OnMessageReceived
		if (status == IMidiMsg::kNoteOn && velocity) {
			if (mKeyStatus[noteNumber] == false) {
				mKeyStatus[noteNumber] = true;
				mNumKeys += 1;
			}
			// A key pressed later overrides any previously pressed key:
			if (noteNumber != mLastNoteNumber) {
				mLastNoteNumber = noteNumber;
				mLastFrequency = noteNumberToFrequency(mLastNoteNumber);
				mLastVelocity = velocity;
				noteOn(noteNumber, velocity);
			}
		}
		else {
			if (mKeyStatus[noteNumber] == true) {
				mKeyStatus[noteNumber] = false;
				mNumKeys -= 1;
				noteOff(noteNumber, mLastVelocity);
			}
			// If the last note was released, nothing should play:
			if (noteNumber == mLastNoteNumber) {
				mLastNoteNumber = -1;
			}
		}
		mMidiQueue.Remove(); 
	}
	mPrevNumKeys = mNumKeys;
	mOffset++;
}
