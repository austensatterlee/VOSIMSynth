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
		int vind = _findIdleVoice();

		m_voiceStack.push_right(vind);
		if (m_voiceMap.find(a_note) == m_voiceMap.end()) {
			// should be okay since nobody else should be trying to access a note that doesn't exist
			m_voiceMap[a_note].unsafe_resize(MAX_VOICES_PER_NOTE);
		}
		m_voiceMap[a_note].push_right(vind);
		m_allVoices[vind]->noteOn(a_note, a_velocity);
		m_numActiveVoices++;
		return vind;
	}

	void VoiceManager::_makeIdle(int a_voiceIndex) {
		if (m_numActiveVoices > 0) {
			shared_ptr<Circuit> voice = m_allVoices[a_voiceIndex];
			int note = voice->getNote();
			int vel = voice->getVelocity();
			if (m_voiceMap.find(note) != m_voiceMap.end()) {
				voice->noteOff(note, vel);
				m_voiceMap[note].remove(a_voiceIndex);
				m_voiceStack.remove(a_voiceIndex);
				m_idleVoiceStack.push_right(a_voiceIndex);
				m_numActiveVoices--;
			}
		}
	}

	int VoiceManager::_findIdleVoice() {
		int vind;
		// Try to find a voice that is already idle
		if (m_numActiveVoices < m_maxVoices) {
			m_idleVoiceStack.pop_left(vind);
			return vind;
		}

		// If none are found, force a voice off the active stack
		vind = _getOldestVoiceIndex();
		_makeIdle(vind);
		m_idleVoiceStack.remove(vind);
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
				m_allVoices[vind]->noteOff(a_noteNumber, a_velocity);
			}
		}
	}

	void VoiceManager::sendControlChange(int a_cc, double a_value) {
		// Send control change to all voices
		for (int i = 0; i < m_maxVoices; i++) {
			m_allVoices[i]->notifyMidiControlChange(a_cc, a_value);
		}
		m_instrument->notifyMidiControlChange(a_cc, a_value);
	}

	void VoiceManager::setMaxVoices(unsigned a_newMax) {
		if (a_newMax < 1)
			a_newMax = 1;

		m_maxVoices = a_newMax;

		// Resize buffers
		m_idleVoiceStack.unsafe_resize(m_maxVoices);
		m_voiceStack.unsafe_resize(m_maxVoices);
		m_garbageList.unsafe_resize(m_maxVoices);

		int vind;
		while (!m_voiceStack.empty()) {
			m_voiceStack.peek(vind);
			_makeIdle(vind);
		}

		while (!m_idleVoiceStack.empty()) {
			m_idleVoiceStack.pop_left(vind);
		}

		while (m_allVoices.size() > a_newMax) {
			m_allVoices.pop_back();
		}

		for (unsigned i = 0; i < m_allVoices.size(); i++) {
			m_allVoices[i] = shared_ptr<Circuit>(static_cast<Circuit*>(m_instrument->clone()));
			m_idleVoiceStack.push_right(i);
		}

		while (m_allVoices.size() < a_newMax) {
			m_idleVoiceStack.push_right(m_allVoices.size());
			m_allVoices.push_back(shared_ptr<Circuit>(static_cast<Circuit*>(m_instrument->clone())));
		}
		m_numActiveVoices = 0;
	}

	void VoiceManager::tick(const double* a_left_input, const double* a_right_input, double* a_left_output, double* a_right_output) {
		m_isPlaying = true;
		_flushActionQueue();

		int nVoices = m_numActiveVoices;
		int voiceInd;
		for (int i = 0; i < nVoices; i++) {
			m_voiceStack.peek(voiceInd, i);
			shared_ptr<Circuit> voice = m_allVoices[voiceInd];
			if (!voice->isActive()) {
				m_garbageList.push_right(voiceInd);
			}
		}

		while (m_garbageList.pop_left(voiceInd)) {
			_makeIdle(voiceInd);
		}

		nVoices = m_numActiveVoices;
		int bufsize = m_bufferSize;

		for (int j = 0; j < bufsize; j++) {
			a_left_output[j] = 0;
			a_right_output[j] = 0;
		}

		for (int i = 0; i < nVoices; i++) {
			m_voiceStack.peek(voiceInd, i);
			shared_ptr<Circuit> voice = m_allVoices[voiceInd];
			for (int j = 0; j < bufsize; j++) {
				voice->m_inputSignals.setChannel(0, a_left_input[j]);
				voice->m_inputSignals.setChannel(1, a_right_input[j]);
				voice->tick();
				voice->reset();
				double left_out = voice->getOutputChannel(0).get();
				double right_out = voice->getOutputChannel(1).get();
				a_left_output[j] += left_out;
				a_right_output[j] += right_out;
			}
		}


		m_tickCount += m_bufferSize;
		m_isPlaying = false;
	}

	int VoiceManager::_getLowestVoiceIndex() const {
		if (m_numActiveVoices > 0) {
			VoiceMap::const_iterator it;
			int voice;
			for (it = m_voiceMap.cbegin(); it != m_voiceMap.cend(); ++it) {
				if (it->second.peek(voice) && m_allVoices[voice]->isActive()) {
					return voice;
				}
			}
		}
		return -1;
	}

	int VoiceManager::_getNewestVoiceIndex() const {
		if (m_numActiveVoices > 0) {
			int vind;
			if (m_voiceStack.peek_right(vind))
				return vind;
		}
		return -1;
	}

	int VoiceManager::_getOldestVoiceIndex() const {
		if (m_numActiveVoices > 0) {
			int vind;
			if (m_voiceStack.peek_left(vind))
				return vind;
		}
		return -1;
	}

	int VoiceManager::_getHighestVoiceIndex() const {
		if (m_numActiveVoices > 0) {
			VoiceMap::const_iterator it;
			int voice;
			for (it = m_voiceMap.cbegin(); it != m_voiceMap.cend(); ++it) {
				if (it->second.peek(voice) && m_allVoices[voice]->isActive()) {
					return voice;
				}
			}
		}
		return -1;
	}

	int VoiceManager::getMaxVoices() const {
		return m_maxVoices;
	}

	vector<int> VoiceManager::getNoteVoices(int a_note) {
		vector<int> voices;
		if (m_voiceMap.find(a_note) != m_voiceMap.end()) {
			const VoiceIndexList& vlist = m_voiceMap[a_note];
			unsigned off = 0;
			int vnum;
			while (vlist.peek(vnum, off++)) {
				if (m_allVoices[vnum]->isActive())
					voices.push_back(vnum);
			}
		}
		return voices;
	}

	bool VoiceManager::isPlaying() const {
		return m_isPlaying;
	}

	void VoiceManager::onIdle() {
		_flushActionQueue();
	}

	int VoiceManager::getNumVoices() const {
		return m_numActiveVoices;
	}

	unsigned VoiceManager::queueAction(ActionMessage* a_msg) {		
		m_queuedActions.push(a_msg);		
		return m_tickCount;
	}

	unsigned VoiceManager::getTickCount() const {
		return m_tickCount;
	}

	void VoiceManager::_flushActionQueue() {
		ActionMessage* msg;
		while (m_queuedActions.pop(msg)) {
			_processAction(msg);
		}
	}

	void VoiceManager::_processAction(ActionMessage* a_msg) {
		Circuit* voice;
		// Apply action to all voices
		for (int i = 0; i <= m_maxVoices; i++) {
			if (i == m_maxVoices) { // Reserve last loop for applying action to prototype voice
				voice = m_instrument.get();
			} else {
				voice = m_allVoices[i].get();
			}
			a_msg->action(voice, i==m_maxVoices, &a_msg->data);
		}
		delete a_msg;
	}

	const Circuit& VoiceManager::getCircuit() const {
		return *m_instrument;
	}

	int VoiceManager::getNumUnits() const {
		return m_instrument->getNumUnits();
	}

	const Unit& VoiceManager::getUnit(int a_id) {
		int voiceInd = _getNewestVoiceIndex();

		if (voiceInd >= 0) {
			return m_allVoices[voiceInd]->getUnit(a_id);
		}
		return m_instrument->getUnit(a_id);
	}

	void VoiceManager::setFs(double a_newFs) {
		// Apply new sampling frequency to all voices
		for (int i = 0; i < m_maxVoices; i++) {
			m_allVoices[i]->setFs(a_newFs);
		}
		m_instrument->setFs(a_newFs);
	}

	void VoiceManager::setBufferSize(int a_blockSize) {
		m_bufferSize = a_blockSize;
	}

	void VoiceManager::setTempo(double a_newTempo) {
		// Apply new tempo to all voices
		for (int i = 0; i < m_maxVoices; i++) {
			m_allVoices[i]->setTempo(a_newTempo);
		}
		m_instrument->setTempo(a_newTempo);
	}
}

