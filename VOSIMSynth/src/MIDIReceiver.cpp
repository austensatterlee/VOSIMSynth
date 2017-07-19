/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/
#include "vosimsynth/MIDIReceiver.h"
#include <vosimlib/VoiceManager.h>
#include <IPlug/IPlugStructs.h>

namespace syn {
    void MIDIReceiver::onMessageReceived(IMidiMsg* midiMessage) {
        IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
        // We're only interested in Note On/Off messages (not CC, pitch, etc.)
        if (status == IMidiMsg::kControlChange || status == IMidiMsg::kNoteOn || status == IMidiMsg::kNoteOff)
            m_midiQueue.Add(midiMessage);
    }

    void MIDIReceiver::Flush(int nFrames) {
        m_midiQueue.Flush(nFrames);
        m_offset = 0;
    }

    void MIDIReceiver::Resize(int blockSize) {
        m_midiQueue.Resize(blockSize);
    }

    void MIDIReceiver::advance() {
        while (!m_midiQueue.Empty()) {
            IMidiMsg* midiMessage = m_midiQueue.Peek();
            if (midiMessage->mOffset > m_offset)
                break;

            IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
            if (status == IMidiMsg::kNoteOff || status == IMidiMsg::kNoteOn) {
                int noteNumber = midiMessage->NoteNumber();
                int velocity = midiMessage->Velocity();
                if (status == IMidiMsg::kNoteOn && velocity > 0) {
                    if (!m_keyStatus[noteNumber]) {
                        m_keyStatus[noteNumber] = true;
                        m_vm.noteOn(noteNumber, velocity);
                    }
                }
                else {
                    m_keyStatus[noteNumber] = false;
                    m_vm.noteOff(noteNumber, velocity);
                }
            }
            else if (status == IMidiMsg::kControlChange) {
                m_vm.sendControlChange(midiMessage->ControlChangeIdx(), midiMessage->ControlChange(midiMessage->ControlChangeIdx()));
            }
            m_midiQueue.Remove();
        }
        m_offset++;
    }
}
