#include "VoiceManager.h"

namespace syn {
    int VoiceManager::_createVoice(int a_note, int a_velocity)
    {
        int vind = _findIdleVoice();
        m_idleVoiceStack.remove(vind);
        m_voiceStack.push_back(vind);
        m_voiceMap[a_note].push_back(vind);
        m_allVoices[vind]->noteOn(a_note, a_velocity);
        m_numVoices++;
        return vind;
    }

    void VoiceManager::_makeIdle()
    {
        if (m_numVoices > 0) {
            int vind = _getOldestVoiceIndex();
            _makeIdle(vind);
        }
    }

    void VoiceManager::_makeIdle(int a_voiceIndex)
    {
        if (m_numVoices > 0) {
            if (m_voiceMap.find(m_allVoices[a_voiceIndex]->getNote()) != m_voiceMap.end()) {
                m_voiceMap[m_allVoices[a_voiceIndex]->getNote()].remove(a_voiceIndex);
                if (m_voiceMap[m_allVoices[a_voiceIndex]->getNote()].empty()) {
                    m_voiceMap.erase(m_allVoices[a_voiceIndex]->getNote());
                }
                m_voiceStack.remove(a_voiceIndex);
                m_idleVoiceStack.push_front(a_voiceIndex);
                m_numVoices--;
            }
        }
    }

    int VoiceManager::_findIdleVoice()
    {
        int voiceIndex = m_idleVoiceStack.front();
        m_idleVoiceStack.pop_front();
        return voiceIndex;
    }

    void VoiceManager::noteOn(int a_noteNumber, int a_velocity)
    {
		m_voiceMutex.lock();
        if (m_numVoices == m_maxVoices - 1) {
            int vind = _getOldestVoiceIndex();
            m_idleVoiceStack.remove(vind);
            Circuit* v = m_allVoices[vind].get();
            m_voiceMap[v->getNote()].remove(vind);
            m_voiceStack.remove(vind);
            v->noteOn(a_noteNumber, a_velocity);
            m_voiceStack.push_back(vind);
            m_voiceMap[a_noteNumber].push_back(vind);
        }
        else if (m_numVoices >= 0) {
            _createVoice(a_noteNumber, a_velocity);
        }
		m_voiceMutex.unlock();
    }

    void VoiceManager::noteOff(int a_noteNumber, int a_velocity)
    {
		m_voiceMutex.lock();
        if (m_voiceMap.find(a_noteNumber) != m_voiceMap.end() && !m_voiceMap[a_noteNumber].empty()) {
            for (VoiceList::iterator v = m_voiceMap[a_noteNumber].begin() ; v != m_voiceMap[a_noteNumber].end() ; ++v) {
                m_allVoices[*v]->noteOff(a_noteNumber, a_velocity);
            }
        }
		m_voiceMutex.unlock();
    }

    void VoiceManager::setMaxVoices(unsigned a_newMax)
    {
		m_voiceMutex.lock();
        if (a_newMax < 1)
            a_newMax = 1;
        m_maxVoices = a_newMax;
        while (m_voiceStack.size() > 0) {
            _makeIdle();
            m_idleVoiceStack.pop_front();
        }

        while (m_idleVoiceStack.size() > 0) {
            m_idleVoiceStack.pop_back();
        }

        while (m_allVoices.size() > a_newMax) {
            m_allVoices.pop_back();
        }

        for (unsigned i = 0 ; i < m_allVoices.size() ; i++) {
            m_allVoices[i] = shared_ptr<Circuit>(static_cast<Circuit*>(m_instrument->clone()));
            m_idleVoiceStack.push_back(i);
        }

        while (m_allVoices.size() < a_newMax) {
            m_allVoices.push_back(shared_ptr<Circuit>(static_cast<Circuit*>(m_instrument->clone())));
            m_idleVoiceStack.push_back(m_allVoices.size() - 1);
        }
        m_numVoices = 0;
		m_voiceMutex.unlock();
    }

    void VoiceManager::tick(const double& a_left_input, const double& a_right_input, double& a_left_output, double& a_right_output)
    {
		m_isPlaying = true;
        _flushActionQueue();
        vector<int> garbage_list;
        a_left_output = 0;
        a_right_output = 0;
        for (VoiceList::const_iterator v = m_voiceStack.begin() ; v != m_voiceStack.end() ; v++) {
            Circuit* voice = m_allVoices[*v].get();
			voice->m_inputSignals.setChannel(0, a_left_input);
			voice->m_inputSignals.setChannel(1, a_right_input);
            if (voice->isActive()) {
                voice->tick();
                voice->reset();
                a_left_output += voice->getOutputChannel(0).get();
                a_right_output += voice->getOutputChannel(1).get();
            }
            else {
                garbage_list.push_back(*v);
            }
        }

		m_voiceMutex.lock();
        for (int i = 0 ; i < garbage_list.size() ; i++) {
            _makeIdle(garbage_list[i]);
        }
		m_voiceMutex.unlock();
		m_tickCount++;
		m_isPlaying = false;
    }

    int VoiceManager::_getLowestVoiceIndex()
    {
        if (m_numVoices > 0) {
            VoiceMap::const_iterator it;
            for (it = m_voiceMap.begin() ; it != m_voiceMap.end() ; it++) {
                if (!it->second.empty() && m_allVoices[it->second.back()]->isActive()) {
                    return it->second.back();
                }
            }
        }
        return -1;
    }

    int VoiceManager::_getNewestVoiceIndex()
    {
        if (m_numVoices > 0) {
            for (VoiceList::const_reverse_iterator it = m_voiceStack.crbegin() ; it != m_voiceStack.crend() ; ++it) {
				if (m_allVoices[*it]->isActive()) {
					return *it;
				}
            }
        }
        return -1;
    }

    int VoiceManager::_getOldestVoiceIndex()
    {
        if (m_numVoices > 0) {
            for (VoiceList::const_iterator it = m_voiceStack.cbegin() ; it != m_voiceStack.cend() ; ++it) {
				if (m_allVoices[*it]->isActive()) {
					return *it;
				}
            }
        }
        return -1;
    }

    int VoiceManager::_getHighestVoiceIndex()
    {
        if (m_numVoices > 0) {
            VoiceMap::const_reverse_iterator it;
            for (it = m_voiceMap.crbegin() ; it != m_voiceMap.crend() ; ++it) {
                if (!it->second.empty() && m_allVoices[it->second.back()]->isActive()) {
                    return it->second.back();
                }
            }
        }
        return -1;
    }

    int VoiceManager::getMaxVoices() const {
	    return m_maxVoices;
    }

	bool VoiceManager::isPlaying() const {
		return m_isPlaying;
    }

	int VoiceManager::getNumVoices() const {
	    return m_numVoices;
    }

	unsigned VoiceManager::queueAction(EMuxAction a_action, const MuxArgs& a_params) {
		if (m_isPlaying) {
			m_queueMutex.lock();
			m_queuedActions.push_back(make_pair(a_action, a_params));
			m_queueMutex.unlock();
		}else {
			doAction(a_action, a_params);
		}
		return m_tickCount;
    }

    void VoiceManager::doAction(EMuxAction a_action, const MuxArgs& a_params)
    {
        _processAction(a_action, a_params);
    }

	unsigned VoiceManager::getTickCount() const {
		return m_tickCount;
    }

	void VoiceManager::_flushActionQueue()
    {
        while (true) {
			if(!m_queuedActions.size()) {
				return;
			}
            EMuxAction action = m_queuedActions.front().first;
            MuxArgs params = m_queuedActions.front().second;

			m_queueMutex.lock();
			m_queuedActions.pop_front();
			m_queueMutex.unlock();

            _processAction(action, params);
        }
    }

    void VoiceManager::_processAction(EMuxAction a_action, const MuxArgs& a_params)
    {
        Circuit* voice;
        // Apply action to all voices
        int numVoices = m_allVoices.size();
        for (int i = 0 ; i <= numVoices ; i++) {
            if (i == m_allVoices.size()) { // Apply action to prototype voice at end of loop
                voice = m_instrument.get();
            } else {
                voice = m_allVoices[i].get();
            }
            switch (a_action) {
                case ModifyParam:
                    voice->setInternalParameter(a_params.id1, a_params.id2, a_params.value);
                    break;
                case ModifyParamNorm:
                    voice->setInternalParameterNorm(a_params.id1, a_params.id2, a_params.value);
                    break;
				case ModifyParamPrecision:
					voice->getUnit_(a_params.id1).getParameter_(a_params.id2).setPrecision(a_params.id3);
					break;
                case DeleteUnit:
                    voice->removeUnit(a_params.id1);
                    break;
                case ConnectInput:
                    voice->connectInputs(a_params.id1, a_params.id2, a_params.id3);
                    break;
                case ConnectOutput:
                    voice->connectOutputs(a_params.id1, a_params.id2, a_params.id3);
                    break;
                case ConnectInternal:
                    voice->connectInternal(a_params.id1, a_params.id2, a_params.id3, a_params.id4);
                    break;
                case DisconnectInput:
                    voice->disconnectInputs(a_params.id1, a_params.id2, a_params.id3);
                    break;
                case DisconnectOutput:
                    voice->disconnectOutputs(a_params.id1, a_params.id2, a_params.id3);
                    break;
                case DisconnectInternal:
                    voice->disconnectInternal(a_params.id1, a_params.id2, a_params.id3, a_params.id4);
                    break;
                default:
                    throw std::domain_error("Invalid queued action");
            }
        }
    }

    const Circuit& VoiceManager::getCircuit() const
    {
        return *m_instrument;
    }

    int VoiceManager::getNumUnits() const
    {
        return m_instrument->getNumUnits();
    }

    const Unit& VoiceManager::getUnit(int a_id)
    {
		m_voiceMutex.lock();
		int voiceInd = _getNewestVoiceIndex();
		m_voiceMutex.unlock();

		if(voiceInd>=0) {
			return m_allVoices[voiceInd]->getUnit(a_id);
		}
        return m_instrument->getUnit(a_id);
    }

	void VoiceManager::setFs(double a_newFs)
    {
        Circuit* voice;
        // Apply action to all voices
        int numVoices = m_allVoices.size();
        for (int i = 0 ; i <= numVoices ; i++) {
            if (i == m_allVoices.size()) { // Apply action to prototype voice at end of loop
                voice = m_instrument.get();
            } else {
                voice = m_allVoices[i].get();
            }
            voice->setFs(a_newFs);
        }
    }

    void VoiceManager::setTempo(double a_newTempo)
    {
        Circuit* voice;
        // Apply action to all voices
        int numVoices = m_allVoices.size();
        for (int i = 0 ; i <= numVoices ; i++) {
            if (i == m_allVoices.size()) { // Apply action to prototype voice at end of loop
                voice = m_instrument.get();
            } else {
                voice = m_allVoices[i].get();
            }
            voice->setTempo(a_newTempo);
        }
    }
}
