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

#include "VoiceManager.h"

namespace syn
{
    int VoiceManager::_createVoice(int a_note, int a_velocity) {
       int vind = _stealIdleVoice();

        m_activeVoices.push_right(vind);
        if (m_voiceMap.find(a_note) == m_voiceMap.end()) {
            m_voiceMap[a_note].resize(m_maxVoices);
        }
        m_voiceMap[a_note].push_right(vind);
        m_circuits[vind].noteOn(a_note, a_velocity);
        m_numActiveVoices++;
        return vind;
    }

    void VoiceManager::_makeIdle(int a_voiceIndex) {
        if (m_numActiveVoices > 0) {
            Circuit& voice = m_circuits[a_voiceIndex];
            int note = voice.note();
            int vel = voice.velocity();
            if (m_voiceMap.find(note) != m_voiceMap.end()) {
                voice.noteOff(note, vel);
                m_voiceMap[note].remove(a_voiceIndex);
                m_activeVoices.remove(a_voiceIndex);
                m_idleVoices.push_right(a_voiceIndex);
                m_numActiveVoices--;
            }
        }
    }

    int VoiceManager::_stealIdleVoice() {
        // Try to find a voice that is already idle
        if (m_numActiveVoices < m_maxVoices) {
            int vind;
            m_idleVoices.pop_left(vind);
            return vind;
        }

        // If none are found, force a voice off the active stack
        int vind = getOldestVoiceIndex();
        _makeIdle(vind);
        m_idleVoices.remove(vind);
        return vind;
    }

    void VoiceManager::noteOn(int a_noteNumber, int a_velocity) {
        _createVoice(a_noteNumber, a_velocity);
    }

    void VoiceManager::noteOff(int a_noteNumber, int a_velocity) {
        if (m_voiceMap.find(a_noteNumber) != m_voiceMap.end()) {
             int vind;
            unsigned off = 0;
            while (m_voiceMap[a_noteNumber].peek(vind, off++)) {
                m_circuits[vind].noteOff(a_noteNumber, a_velocity);
            }
        }
    }

    void VoiceManager::sendControlChange(int a_cc, double a_value) {
        // Send control change to all voices
        for (int i = 0; i < m_maxVoices; i++) {
            m_circuits[i].notifyMidiControlChange(a_cc, a_value);
        }
        m_instrument.notifyMidiControlChange(a_cc, a_value);
    }

    void VoiceManager::setMaxVoices(unsigned a_newMax) {
        if (a_newMax < 1)
            a_newMax = 1;
        if (a_newMax > MAX_VOICES)
            a_newMax = MAX_VOICES;

        m_maxVoices = a_newMax;
        // Resize buffers
        m_idleVoices.resize(m_maxVoices);
        m_activeVoices.resize(m_maxVoices);
        m_garbageList.resize(m_maxVoices);

        int vind;
         while (!m_activeVoices.empty()) {
            m_activeVoices.peek(vind);
            _makeIdle(vind);
        }
        m_numActiveVoices = 0;

        while (!m_idleVoices.empty()) {
                m_idleVoices.pop_left(vind);
        }

        while (m_circuits.size() > a_newMax) {
            m_circuits.pop_back();
        }

        for (unsigned i = 0; i < m_circuits.size(); i++) {
            m_circuits[i] = Circuit(m_instrument);
            m_idleVoices.push_right(i);
        }

        while (m_circuits.size() < a_newMax) {
            m_idleVoices.push_right(m_circuits.size());
            m_circuits.push_back(Circuit(m_instrument));
        }
    }

    void VoiceManager::tick(const double* a_left_input, const double* a_right_input, double* a_left_output, double* a_right_output) {
        _flushActionQueue();

        int vind;

        for (int i = 0; i < m_numActiveVoices; i++) {
            m_activeVoices.peek(vind, i);
            Circuit& voice = m_circuits[vind];
            if (!voice.isActive()) {
                m_garbageList.push_right(vind);
            }
        }

        while (m_garbageList.pop_left(vind)) {
            _makeIdle(vind);
        }

        for (int j = 0; j < m_bufferSize; j++) {
            a_left_output[j] = 0;
            a_right_output[j] = 0;
        }

        for (int sample = 0; sample < m_bufferSize; sample += m_internalBufferSize) {
            for (int i = 0; i < m_numActiveVoices; i++) {
                m_activeVoices.peek(vind, i);
                Circuit& voice = m_circuits[vind];
                voice.connectInput(0, a_left_input);
                voice.connectInput(1, a_right_input);
                voice.tick();
                for (int j = 0; j < m_internalBufferSize; j++) {
                    a_left_output[sample + j] += voice.readOutput(0, j);
                    a_right_output[sample + j] += voice.readOutput(1, j);
                }
            }
        }
        m_tickCount += m_bufferSize;
    }

    int VoiceManager::getLowestVoiceIndex() const {
        if (m_numActiveVoices > 0) {
            VoiceMap::const_iterator it;
            int voice;
            for (it = m_voiceMap.cbegin(); it != m_voiceMap.cend(); ++it) {
                if (it->second.peek(voice) && m_circuits[voice].isActive()) {
                    return voice;
                }         
            }
        }
        return -1;
    }

    int VoiceManager::getNewestVoiceIndex() const {
        if (m_numActiveVoices > 0) {
            int vind;
            if (m_activeVoices.peek_right(vind))
                return vind;
        }
        return -1;
    }

    int VoiceManager::getOldestVoiceIndex() const {
        if (m_numActiveVoices > 0) {
            int vind;
            if (m_activeVoices.peek_left(vind))
                return vind;
        }
        return -1;
    }

    int VoiceManager::getHighestVoiceIndex() const {
        if (m_numActiveVoices > 0) {
            VoiceMap::const_iterator it;
            int voice;
            for (it = m_voiceMap.cbegin(); it != m_voiceMap.cend(); ++it) {
                if (it->second.peek(voice) && m_circuits[voice].isActive()) {
                    return voice;
                }
            }
        }
        return -1;
    }

    int VoiceManager::getMaxVoices() const {
        return m_maxVoices;
    }

    void VoiceManager::onIdle() {
        if (!m_numActiveVoices)
            _flushActionQueue();
    }

    int VoiceManager::getNumVoices() const {
        return m_numActiveVoices;
    }

    unsigned VoiceManager::queueAction(RTMessage* a_msg) {
        m_queuedActions.push(a_msg);
        return m_tickCount;
    }

    unsigned VoiceManager::getTickCount() const {
        return m_tickCount;
    }

    void VoiceManager::_flushActionQueue() {
        RTMessage* msg;
        while (m_queuedActions.pop(msg)) {
            _processAction(msg);
            delete msg;
        }
    }

    void VoiceManager::_processAction(RTMessage* a_msg) {
        // Apply action to all voices
        for (int i = 0; i < m_maxVoices; i++) {
            a_msg->action(&m_circuits[i], false, &a_msg->data);
        }
        a_msg->action(&m_instrument, true, &a_msg->data);
    }

    Circuit* VoiceManager::getPrototypeCircuit() {
        return &m_instrument;
    }

    const Circuit* VoiceManager::getPrototypeCircuit() const {
        return &m_instrument;
    }

    Circuit* VoiceManager::getVoiceCircuit(int a_voiceId)
    {
        return &m_circuits[a_voiceId];
    }

    const Circuit* VoiceManager::getVoiceCircuit(int a_voiceId) const
    {
        return &m_circuits[a_voiceId];
    }

    void VoiceManager::setPrototypeCircuit(const Circuit& a_circ) {
        m_instrument = Circuit{ a_circ };
        setMaxVoices(m_maxVoices);
    }

    Unit& VoiceManager::getUnit(int a_id, int a_voiceInd) {
        if (a_voiceInd >= 0) {
            return m_circuits[a_voiceInd].getUnit(a_id);
        }
        return m_instrument.getUnit(a_id);
    }

    const Unit& VoiceManager::getUnit(int a_id, int a_voiceInd) const {
        if (a_voiceInd >= 0) {
            return m_circuits[a_voiceInd].getUnit(a_id);
        }
        return m_instrument.getUnit(a_id);
    }

    void VoiceManager::setFs(double a_newFs) {
        // Apply new sampling frequency to all voices
        for (int i = 0; i < m_maxVoices; i++) {
            m_circuits[i].setFs(a_newFs);
        }
        m_instrument.setFs(a_newFs);
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
        for (int i = 0; i < m_maxVoices; i++) {
            m_circuits[i].setBufferSize(m_internalBufferSize);
        }
        m_instrument.setBufferSize(m_internalBufferSize);
    }

    void VoiceManager::setTempo(double a_newTempo) {
        // Apply new tempo to all voices
        for (int i = 0; i < m_maxVoices; i++) {
            m_circuits[i].setTempo(a_newTempo);
        }
        m_instrument.setTempo(a_newTempo);
    }
}