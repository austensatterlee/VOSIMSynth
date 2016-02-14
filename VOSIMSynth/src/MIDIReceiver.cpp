#include "MIDIReceiver.h"

void MIDIReceiver::onMessageReceived(IMidiMsg* midiMessage)
{
  IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
  // We're only interested in Note On/Off messages (not CC, pitch, etc.)
  if (status == IMidiMsg::kControlChange || status == IMidiMsg::kNoteOn || status == IMidiMsg::kNoteOff)
    m_midiQueue.Add(midiMessage);
}

void MIDIReceiver::advance()
{
  while (!m_midiQueue.Empty())
  {
    IMidiMsg* midiMessage = m_midiQueue.Peek();
    if (midiMessage->mOffset > m_offset) break;

    IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
    if (status == IMidiMsg::kNoteOff || status == IMidiMsg::kNoteOn)
    {
      int noteNumber = midiMessage->NoteNumber();
      int velocity = midiMessage->Velocity();
      if (status == IMidiMsg::kNoteOn && velocity > 0)
      {
        if (!m_keyStatus[noteNumber])
        {
          m_keyStatus[noteNumber] = true;
          m_vm->noteOn(noteNumber, velocity);
        }
      }
      else
      {
        m_keyStatus[noteNumber] = false;
        m_vm->noteOff(noteNumber, velocity);
      }
    }
    else if (status == IMidiMsg::kControlChange)
    {
      //sendControlChange(*midiMessage);
    }
    m_midiQueue.Remove();
  }
  m_offset++;
}