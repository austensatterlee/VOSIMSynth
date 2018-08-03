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
#include "vosimlib/VoiceManager.h"
#include "vosimlib/Command.h"

namespace syn
{
    void VoiceManager::setFs(double a_newFs) {
        // Apply new sampling frequency to all voices
        for (int i = 0; i < m_voices.size(); i++) {
            m_voices[i].setFs(a_newFs);
        }
        m_instrument.setFs(a_newFs);
    }

    void VoiceManager::setTempo(double a_newTempo) {
        // Apply new tempo to all voices
        for (int i = 0; i < m_voices.size(); i++) {
            m_voices[i].setTempo(a_newTempo);
        }
        m_instrument.setTempo(a_newTempo);
    }

    void VoiceManager::noteOn(int a_noteNumber, int a_velocity) {
        int currVoiceIndex = m_lastVoiceIndex;
        int bestVoiceIndex = -1;
        int bestVoiceScore = 0;
        do {
            auto& voice = m_voices[currVoiceIndex];
            if (!voice.isActive()) {
                voice.noteOn(a_noteNumber, a_velocity);
                m_lastVoiceIndex = currVoiceIndex;
                m_voiceBirths[currVoiceIndex] = m_voiceTicks;
                m_voiceTicks++;
                return;
            }

            int voiceAge = m_voiceTicks - m_voiceBirths[currVoiceIndex];
            int currVoiceScore = 0;
            switch(m_voiceStealingPolicy) {
            case Highest:
                currVoiceScore -= voice.note();
                break;
            case Lowest:
                currVoiceScore += voice.note();
                break;
            case Newest:
                currVoiceScore -= voiceAge;
                break;
            case Oldest:
            default:
                currVoiceScore += voiceAge;
                break;
            }
            currVoiceScore += voice.isNoteOn() ? 0 : 1;

            if(currVoiceScore>bestVoiceScore || bestVoiceIndex==-1) {
                bestVoiceIndex = currVoiceIndex;
                bestVoiceScore = currVoiceScore;
            }

            currVoiceIndex++;
            if (currVoiceIndex == m_voices.size())
                currVoiceIndex = 0;
        } while (currVoiceIndex != m_lastVoiceIndex);

        // TODO: If not legato, release voice for a few ms before retriggering
        m_voices[bestVoiceIndex].noteOn(a_noteNumber, a_velocity);
        m_lastVoiceIndex = bestVoiceIndex;
        m_voiceBirths[bestVoiceIndex] = m_voiceTicks;
        m_voiceTicks++;
    }

    void VoiceManager::noteOff(int a_noteNumber) {
        for(auto& voice : m_voices) {
            if(voice.note() == a_noteNumber)
                voice.noteOff();
        }
    }

    void VoiceManager::sendControlChange(int a_cc, double a_value) {
        // Send control change to all voices
        for (int i = 0; i < m_voices.size(); i++) {
            m_voices[i].notifyMidiControlChange(a_cc, a_value);
        }
        m_instrument.notifyMidiControlChange(a_cc, a_value);
    }

    void VoiceManager::sendPitchWheelChange(double a_value) {
        // Send pitch wheel change to all voices
        for (int i = 0; i < m_voices.size(); i++) {
            m_voices[i].notifyPitchWheelChange(a_value);
        }
        m_instrument.notifyPitchWheelChange(a_value);
    }

    void VoiceManager::setMaxVoices(int a_newMax) {
        if (a_newMax < 1)
            a_newMax = 1;
        if (a_newMax > MAX_VOICES)
            a_newMax = MAX_VOICES;

        m_voices.clear();
        m_voiceBirths.clear();
        m_lastVoiceIndex = 0;
        m_voiceTicks = 0;

        // Construct new voices
        m_voices.resize(a_newMax);
        m_voiceBirths.resize(a_newMax, -1);

        for(int i=0;i<a_newMax;i++)
        {
            m_voices[i] = m_instrument;
            m_voices[i].setVoiceIndex(a_newMax>1 ? (i+1) * 1.0 / a_newMax : 1.0);
        }
    }

    vector<int> VoiceManager::getActiveVoiceIndices() const
    {
        vector<int> voiceIndices;
        for (int i = 0; i < m_voices.size(); i++) {
            if (m_voices[i].isActive())
                voiceIndices.push_back(i);
        }
        return voiceIndices;
    }

    vector<int> VoiceManager::getReleasedVoiceIndices() const {
        vector<int> voiceIndices;
        for (int i = 0; i < m_voices.size(); i++) {
            if (m_voices[i].isActive() && !m_voices[i].isNoteOn())
                voiceIndices.push_back(i);
        }
        return voiceIndices;
    }

    vector<int> VoiceManager::getIdleVoiceIndices() const {
        vector<int> voiceIndices;
        for (int i = 0; i < m_voices.size(); i++) {
            if (!m_voices[i].isActive())
                voiceIndices.push_back(i);
        }
        return voiceIndices;
    }

    void VoiceManager::tick(const double* a_left_input, const double* a_right_input, double* a_left_output, double* a_right_output) {
        _flushActionQueue();

        for (int j = 0; j < m_bufferSize; j++) {
            a_left_output[j] = 0;
            a_right_output[j] = 0;
        }

        for (auto& voice : m_voices) {
            if (voice.isActive()) {
                for (int sample = 0; sample < m_bufferSize; sample += m_internalBufferSize) {
                    ReadOnlyBuffer<double> left{a_left_input}, right{a_right_input};
                    voice.connectInput(0, left);
                    voice.connectInput(1, right);
                    voice.tick();
                    for (int j = 0; j < m_internalBufferSize; j++) {
                        a_left_output[sample + j] += voice.readOutput(0, j);
                        a_right_output[sample + j] += voice.readOutput(1, j);
                    }
                }
            }
        }
    }

    int VoiceManager::getNewestVoiceIndex() const {
        return m_lastVoiceIndex;
    }

    int VoiceManager::getMaxVoices() const {
        return int(m_voices.size());
    }

    void VoiceManager::onIdle() {
        _flushActionQueue();
    }

    bool VoiceManager::queueAction(Command* a_msg) {
        if(!m_queuedActions.write_available()){
            return false;
        }
        m_queuedActions.push(a_msg);
        return true;
    }

    void VoiceManager::_flushActionQueue() {
        Command* msg;
        while (m_queuedActions.pop(msg)) {
            (*msg)();
            delete msg;
        }
    }

    Circuit& VoiceManager::getPrototypeCircuit() {
        return m_instrument;
    }

    const Circuit& VoiceManager::getPrototypeCircuit() const {
        return m_instrument;
    }

    Circuit& VoiceManager::getVoiceCircuit(int a_voiceId)
    {
        return a_voiceId<0 ? m_instrument : m_voices[a_voiceId];
    }

    const Circuit& VoiceManager::getVoiceCircuit(int a_voiceId) const
    {
        return a_voiceId<0 ? m_instrument : m_voices[a_voiceId];
    }

    void VoiceManager::setPrototypeCircuit(const Circuit& a_circ) {
        m_instrument = Circuit{ a_circ };
        setMaxVoices(int(m_voices.size()));
    }

    Unit& VoiceManager::getUnit(int a_id, int a_voiceInd) {
        if (a_voiceInd >= 0) {
            return m_voices[a_voiceInd].getUnit(a_id);
        }
        return m_instrument.getUnit(a_id);
    }

    const Unit& VoiceManager::getUnit(int a_id, int a_voiceInd) const {
        if (a_voiceInd >= 0) {
            return m_voices[a_voiceInd].getUnit(a_id);
        }
        return m_instrument.getUnit(a_id);
    }

    void VoiceManager::setBufferSize(int a_bufferSize) {
        m_bufferSize = a_bufferSize > 0 ? a_bufferSize : 1;
        setInternalBufferSize(m_internalBufferSize);
    }

    void VoiceManager::setInternalBufferSize(int a_internalBufferSize)
    {
        // Force internal buffer size to a multiple of the final buffer size
        a_internalBufferSize = a_internalBufferSize > 0 ? a_internalBufferSize : 1;
        int u = a_internalBufferSize;
        int d = a_internalBufferSize;
        m_internalBufferSize = m_bufferSize;
        bool stop = false;
        while (!stop)
        {
            if (u < m_bufferSize) {
                if (m_bufferSize%u == 0) {
                    m_internalBufferSize = u;
                    break;
                }
                u++;
            }
            if (d > 0) {
                if (m_bufferSize%d == 0) {
                    m_internalBufferSize = d;
                    break;
                }
                d--;
            }
            stop = !(u < m_bufferSize || d>0);
        }

        // Propogate the new buffer size   
        for (int i = 0; i < m_voices.size(); i++) {
            m_voices[i].setBufferSize(m_internalBufferSize);
        }
        m_instrument.setBufferSize(m_internalBufferSize);
    }
}