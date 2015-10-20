#include "MIDIReceiver.h"

void MIDIReceiver::onMessageReceived(IMidiMsg* midiMessage)
{
  IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
  // We're only interested in Note On/Off messages (not CC, pitch, etc.)
  if (status == IMidiMsg::kControlChange || status == IMidiMsg::kNoteOn || status == IMidiMsg::kNoteOff)
    mMidiQueue.Add(midiMessage);
}

void MIDIReceiver::advance()
{
  while (!mMidiQueue.Empty())
  {
    IMidiMsg* midiMessage = mMidiQueue.Peek();
    if (midiMessage->mOffset > mOffset) break;

    IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
    if (status == IMidiMsg::kNoteOff || status == IMidiMsg::kNoteOn)
    {
      int noteNumber = midiMessage->NoteNumber();
      int velocity = midiMessage->Velocity();
      if (status == IMidiMsg::kNoteOn && velocity>0)
      {
        if (mKeyStatus[noteNumber] == false)
        {
          mKeyStatus[noteNumber] = true;
          mNumKeys += 1;
          noteOn(noteNumber, velocity);
        }
      }
      else
      {
        mKeyStatus[noteNumber] = false;
        mNumKeys -= 1;
        noteOff(noteNumber, velocity);
      }
    }
    else if (status == IMidiMsg::kControlChange)
    {
      sendControlChange(*midiMessage);
    }
    mMidiQueue.Remove();
  }
  mOffset++;
}
