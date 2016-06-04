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

#ifndef __VOICEMANAGER__
#define __VOICEMANAGER__

#include "Circuit.h"
#include "Unit.h"
#include "UnitFactory.h"
#include "AtomicContainers.h"
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/policies.hpp>
#include <map>
#include <memory>
#include <Containers.h>

#define MAX_VOICEMANAGER_MSG_QUEUE_SIZE 64

using std::map;
using std::string;
using std::shared_ptr;
using boost::lockfree::spsc_queue;
using boost::lockfree::capacity;

namespace syn
{
	/**
	 * Used to pass messages to a voice manager via VoiceManager::queueAction
	 * The action function pointer will be called once for each voice (with the corresponding circuit passed as the first parameter).
	 * On the last voice, the second parameter will be true.
	 * The third parameter is a pointer to the ByteChunk stored in this structure, which is useful for passing arguments.
	 */
	struct RTMessage
	{
		void(*action)(Circuit*, bool, ByteChunk*);
		ByteChunk data;
	};

	class VoiceManager
	{
	public:
		enum VoiceSelectionPolicy
		{
			LOW,
			HIGH,
			OLD,
			NEW
		};

	public:
		VoiceManager(shared_ptr<Circuit> a_proto, UnitFactory* a_factory) :
			m_queuedActions(MAX_VOICEMANAGER_MSG_QUEUE_SIZE),
			m_numActiveVoices(0),
			m_maxVoices(0),
			m_bufferSize(1),
			m_tickCount(0),
			m_voiceStack(0),
			m_idleVoiceStack(0),
			m_garbageList(0),
			m_instrument(a_proto),
			m_factory(a_factory)
		{ };

		~VoiceManager() {
			m_allVoices.clear();
		}

		void MSFASTCALL tick(const double* a_left_input, const double* a_right_input, double* a_left_output, double* a_right_output) GCCFASTCALL;

		/**
		 * Safely queue a function to be called on the real-time thread in between samples.
		 */
		unsigned queueAction(RTMessage* a_action);

		unsigned getTickCount() const;

		void setFs(double a_newFs);

		void setBufferSize(int a_blockSize);

		void setTempo(double a_newTempo);

		void noteOn(int a_noteNumber, int a_velocity);

		void noteOff(int a_noteNumber, int a_velocity);

		void sendControlChange(int a_cc, double a_newvalue);

		void setMaxVoices(unsigned a_newMax);

		int getNumVoices() const;

		int getMaxVoices() const;

		int getLowestVoiceIndex() const;

		int getNewestVoiceIndex() const;

		int getOldestVoiceIndex() const;

		int getHighestVoiceIndex() const;

		/**
		 * Get the voice numbers that are currently playing the given note
		 */
		vector<int> getNoteVoices(int a_note);

		void onIdle();

		/**
		 * Retrieves a unit from a specific voice circuit.
		 * If the given voice id is negative, the unit is retrieved from the prototype circuit.
		 */
		const Unit& getUnit(int a_id, int a_voiceId = -1);

		int getNumUnits() const;

		shared_ptr<const Circuit> getPrototypeCircuit() const;

		shared_ptr<const Circuit> getVoiceCircuit(int a_voiceId) const;

		void save(ByteChunk* a_data) const;
		int load(ByteChunk* a_data, int startPos);

	private:
		/**
		 * Processes all actions from the action queue
		 */
		void _flushActionQueue();

		/**
		 * Processes the next action from the action queue
		 */
		void _processAction(RTMessage* a_msg);

		int _createVoice(int a_note, int a_velocity);

		void _makeIdle(int a_voiceIndex);

		int _findIdleVoice();

	private:
		typedef AtomicQueue<int> VoiceIndexList;
		typedef map<int, AtomicQueue<int>> VoiceMap;

		spsc_queue<RTMessage*> m_queuedActions;

		unsigned m_numActiveVoices; /// Number of active voices
		unsigned m_maxVoices; /// Total number of voices (idle voices + active voices)
		unsigned m_bufferSize;
		unsigned m_tickCount;

		VoiceMap m_voiceMap;
		VoiceIndexList m_voiceStack;
		VoiceIndexList m_idleVoiceStack;
		VoiceIndexList m_garbageList; /// pre-allocated storage for collecting idle voices during audio processing
		vector<shared_ptr<Circuit>> m_allVoices;
		shared_ptr<Circuit> m_instrument;
		UnitFactory* m_factory;
	};
}
#endif
