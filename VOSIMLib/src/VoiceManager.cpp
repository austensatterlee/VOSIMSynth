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
    int VoiceManager::_createVoice(int a_note, int a_velocity) {
       int vind = _stealIdleVoice();

        m_activeVoices.push_back(vind);
        if (m_voiceMap.find(a_note) == m_voiceMap.end()) {
            m_voiceMap[a_note].reserve(m_circuits.size());
        }
        m_voiceMap[a_note].push_back(vind);
        m_circuits[vind].noteOn(a_note, a_velocity);
        return vind;
    }

    void VoiceManager::_makeIdle(int a_voiceIndex) {
        auto activeIt = std::find(m_activeVoices.begin(), m_activeVoices.end(), a_voiceIndex);
        if(activeIt!=m_activeVoices.end()){
            Circuit& voice = m_circuits[a_voiceIndex];
            int note = voice.note();
            int vel = voice.velocity();

            
            // If the voice has been sent a note off signal, remove from released voices
            auto releasedIt = std::find(m_releasedVoices.begin(), m_releasedVoices.end(), a_voiceIndex);
            if(releasedIt!=m_releasedVoices.end())
                m_releasedVoices.erase(releasedIt);

            // If the voice has not been sent a note off signal, perform reset unless legato is on
            if(voice.isNoteOn() && !m_legato) {
                voice.noteOff(note);
            }

            // Remove from active voice list
            m_activeVoices.erase(activeIt);
            // Remove from note map
            auto mapIt = std::find(m_voiceMap[note].begin(), m_voiceMap[note].end(), a_voiceIndex);
            assert(mapIt!=m_voiceMap[note].end());
            m_voiceMap[note].erase(mapIt);
            // Remove note from note map if no more voices exist there
            if(m_voiceMap[note].empty())
                m_voiceMap.erase(note);
            // Add voice to idle voices
            m_idleVoices.push_back(a_voiceIndex);
        }
    }

    int VoiceManager::_stealIdleVoice() {
        // Try to find a voice that is already idle
        if (!m_idleVoices.empty()) {
            const int vind = m_idleVoices.front();
            m_idleVoices.erase(m_idleVoices.begin());
            return vind;
        }

        // If none are found, force a voice off the active stack
        int vind;
        switch(m_voiceStealingPolicy)
        {
        case Oldest:
            vind = getOldestVoiceID(true);
            break;
        case Newest:
            vind = getNewestVoiceID(true);
            break;
        case Lowest:
            vind = getLowestVoiceID(true);
            break;
        case Highest:
            vind = getHighestVoiceID(true);
            break;
        default:
            vind = getOldestVoiceID(true);
            break;
        }
        _makeIdle(vind);
        m_idleVoices.erase(std::find(m_idleVoices.begin(), m_idleVoices.end(), vind));
        return vind;
    }

    void VoiceManager::noteOn(int a_noteNumber, int a_velocity) {
        _createVoice(a_noteNumber, a_velocity);
    }

    void VoiceManager::noteOff(int a_noteNumber) {
        if (m_voiceMap.find(a_noteNumber) != m_voiceMap.end()) {
            for (int vind : m_voiceMap[a_noteNumber]) {
                m_circuits[vind].noteOff(a_noteNumber);
                if(std::find(m_releasedVoices.begin(), m_releasedVoices.end(), vind)==m_releasedVoices.end())
                    m_releasedVoices.push_back(vind);
            }
        }
    }

    void VoiceManager::sendControlChange(int a_cc, double a_value) {
        // Send control change to all voices
        for (int i = 0; i < m_circuits.size(); i++) {
            m_circuits[i].notifyMidiControlChange(a_cc, a_value);
        }
        m_instrument.notifyMidiControlChange(a_cc, a_value);
    }

    void VoiceManager::sendPitchWheelChange(double a_value) {
        // Send pitch wheel change to all voices
        for (int i = 0; i < m_circuits.size(); i++) {
            m_circuits[i].notifyPitchWheelChange(a_value);
        }
        m_instrument.notifyPitchWheelChange(a_value);
    }

    void VoiceManager::setMaxVoices(int a_newMax) {
        if (a_newMax < 1)
            a_newMax = 1;
        if (a_newMax > MAX_VOICES)
            a_newMax = MAX_VOICES;

        // Return all voices to idle state
        for(int i=0; i<m_circuits.size(); i++)
        {
            _makeIdle(i);
        }

        // Clear voice lists
        m_idleVoices.clear();
        m_activeVoices.clear();
        m_releasedVoices.clear();
        m_voiceMap.clear();
        m_circuits.clear();

        // Resize voice lists
        m_idleVoices.reserve(a_newMax);
        m_activeVoices.reserve(a_newMax);
        m_garbageList.reserve(a_newMax);        
        m_releasedVoices.reserve(a_newMax);
        // Construct new voices
        m_circuits.resize(a_newMax);

        // Add new voices to the idle list
        for(int i=0;i<a_newMax;i++)
        {
            m_circuits[i] = m_instrument;
            m_circuits[i].setVoiceIndex(a_newMax>1 ? (i+1) * 1.0 / a_newMax : 1.0);
            m_idleVoices.push_back(i);
        }
    }

    vector<int> VoiceManager::getActiveVoiceIndices() const
    {
        vector<int> voiceIndices(m_activeVoices.size());
        for(int i=0; i<m_activeVoices.size(); i++){
            voiceIndices[i] = m_activeVoices.at(i);
        }
        return voiceIndices;
    }

    vector<int> VoiceManager::getReleasedVoiceIndices() const {
        vector<int> voiceIndices(m_releasedVoices.size());
        for(int i=0; i<m_releasedVoices.size(); i++){
            voiceIndices[i] = m_releasedVoices.at(i);
        }
        return voiceIndices;                
    }

    vector<int> VoiceManager::getIdleVoiceIndices() const {
        vector<int> voiceIndices(m_idleVoices.size());
        for(int i=0; i<m_idleVoices.size(); i++){
            voiceIndices[i] = m_idleVoices.at(i);
        }
        return voiceIndices;          
    }

    void VoiceManager::tick(const double* a_left_input, const double* a_right_input, double* a_left_output, double* a_right_output) {
        _flushActionQueue();

        int vind;

        for (int i = 0; i < m_activeVoices.size(); i++) {
            vind = m_activeVoices.at(i);
            Circuit& voice = m_circuits[vind];
            if (!voice.isActive()) {
                m_garbageList.push_back(vind);
            }
        }

        while (!m_garbageList.empty()) {
            vind = m_garbageList.back();
            _makeIdle(vind);
            m_garbageList.pop_back();
        }

        for (int j = 0; j < m_bufferSize; j++) {
            a_left_output[j] = 0;
            a_right_output[j] = 0;
        }

        for (int i = 0; i < m_activeVoices.size(); i++) {
            for (int sample = 0; sample < m_bufferSize; sample += m_internalBufferSize) {
                vind = m_activeVoices.at(i);
                Circuit& voice = m_circuits[vind];
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
        m_tickCount += m_bufferSize;
    }

    int VoiceManager::getLowestVoiceID(bool a_preferReleased) const {
        if (!m_releasedVoices.empty() && a_preferReleased) {
            bool first = true;
            int minNote = 0;
            int minVInd = 0;
            for(auto vind : m_releasedVoices)
            {
                const Circuit& voice = m_circuits[vind];
                if(first || voice.note()<minNote)
                {
                    minNote = voice.note();
                    minVInd = vind;
                    first = false;
                }
            }
            return minVInd;
        }
        else if (!m_activeVoices.empty())
        {
            for (auto it = m_voiceMap.crbegin(); it != m_voiceMap.crend(); ++it) {
                if (!it->second.empty() && m_circuits[it->second.front()].isActive()) {
                    return it->second.front();
                }
            }
        }
        return -1;
    }

    int VoiceManager::getNewestVoiceID(bool a_preferReleased) const {
        if (!m_releasedVoices.empty() && a_preferReleased) {
            bool first = true;
            int maxIndex = 0;
            int maxVInd = 0;
            for(auto vind : m_releasedVoices)
            {
                int activeVoiceIndex = std::find(m_activeVoices.begin(), m_activeVoices.end(), vind) - m_activeVoices.begin();
                if(first || activeVoiceIndex>maxIndex)
                {
                    maxIndex = activeVoiceIndex;
                    maxVInd = vind;
                    first = false;
                }
            }
            return maxVInd;
        }
        else if (!m_activeVoices.empty())
        {
            return m_activeVoices.back();
        }
        return -1;
    }

    int VoiceManager::getOldestVoiceID(bool a_preferReleased) const {
        if (!m_releasedVoices.empty() && a_preferReleased) {
            bool first = true;
            int minIndex = 0;
            int minVInd = 0;
            for(auto vind : m_releasedVoices)
            {
                int activeVoiceIndex = std::find(m_activeVoices.begin(), m_activeVoices.end(), vind) - m_activeVoices.begin();
                if(first || activeVoiceIndex<minIndex)
                {
                    minIndex = activeVoiceIndex;
                    minVInd = vind;
                    first = false;
                }
            }
            return minVInd;
        }
        else if (!m_activeVoices.empty())
        {
            return m_activeVoices.front();
        }
        return -1;
    }

    int VoiceManager::getHighestVoiceID(bool a_preferReleased) const {
        if (!m_releasedVoices.empty() && a_preferReleased) {
            bool first = true;
            int maxNote = 0;
            int maxVInd = 0;
            for(auto vind : m_releasedVoices)
            {
                const Circuit& voice = m_circuits[vind];
                if(first || voice.note()>maxNote)
                {
                    maxNote = voice.note();
                    maxVInd = vind;
                    first = false;
                }
            }
            return maxVInd;
        }
        else if (!m_activeVoices.empty())
        {
            for (auto it = m_voiceMap.cbegin(); it != m_voiceMap.cend(); ++it) {
                if (!it->second.empty() && m_circuits[it->second.front()].isActive()) {
                    return it->second.front();
                }
            }
        }
        return -1;
    }

    int VoiceManager::getMaxVoices() const {
        return int(m_circuits.size());
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

    unsigned VoiceManager::getTickCount() const {
        return m_tickCount;
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
        return a_voiceId<0 ? m_instrument : m_circuits[a_voiceId];
    }

    const Circuit& VoiceManager::getVoiceCircuit(int a_voiceId) const
    {
        return a_voiceId<0 ? m_instrument : m_circuits[a_voiceId];
    }

    void VoiceManager::setPrototypeCircuit(const Circuit& a_circ) {
        m_instrument = Circuit{ a_circ };
        setMaxVoices(int(m_circuits.size()));
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
        for (int i = 0; i < m_circuits.size(); i++) {
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
        for (int i = 0; i < m_circuits.size(); i++) {
            m_circuits[i].setBufferSize(m_internalBufferSize);
        }
        m_instrument.setBufferSize(m_internalBufferSize);
    }

    void VoiceManager::setTempo(double a_newTempo) {
        // Apply new tempo to all voices
        for (int i = 0; i < m_circuits.size(); i++) {
            m_circuits[i].setTempo(a_newTempo);
        }
        m_instrument.setTempo(a_newTempo);
    }
}