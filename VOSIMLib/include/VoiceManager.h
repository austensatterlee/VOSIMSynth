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

#define MAX_QUEUE_SIZE 64
#define MAX_VOICES_PER_NOTE 8

using std::map;
using std::string;
using std::shared_ptr;
using boost::lockfree::spsc_queue;
using boost::lockfree::capacity;

namespace syn
{
	/* Actions that should be queued and processed in between samples */
	struct ActionMessage
	{
		void(*action)(Circuit*, bool, ByteChunk*);
		ByteChunk data;
	};

	class VoiceManager
	{
	public:
		VoiceManager(shared_ptr<Circuit> a_proto, UnitFactory* a_factory) :
			m_queuedActions(MAX_QUEUE_SIZE),
			m_numActiveVoices(0),
			m_maxVoices(0),
			m_bufferSize(1),
			m_tickCount(0),
			m_isPlaying(false),
			m_voiceStack(0),
			m_idleVoiceStack(0),
			m_garbageList(0),
			m_instrument(a_proto),
			m_factory(a_factory) { };

		~VoiceManager() {
			m_allVoices.clear();
		}

		void MSFASTCALL tick(const double* a_left_input, const double* a_right_input, double* a_left_output, double* a_right_output) GCCFASTCALL;

		unsigned queueAction(ActionMessage* a_action);

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

		/**
		 * Get the voice numbers that are currently playing the given note
		 */
		vector<int> getNoteVoices(int a_note);

		bool isPlaying() const;

		void onIdle();

		const Unit& getUnit(int a_id);

		int getNumUnits() const;

		const Circuit& getCircuit() const;

		/** 
		 * \todo: This method is not thread safe. Instead message passing should be reimplemented in a way that allows the NRT
		 * thread to be notified when results are available (e.g. to retrieve the unitid). Perhaps a struct with an arbitrary chunk of parameter data,
		 * a function pointer to the method doing work in the realtime thread, and a function pointer to a method that queues a notification of completion
		 * for the NRT thread to poll.
		 */
		template <typename T>
		int addUnit(T a_prototypeId);

	private:
		/**
		 * Processes all actions from the action queue
		 */
		void _flushActionQueue();

		/**
		 * Processes the next action from the action queue
		 */
		void _processAction(ActionMessage* a_msg);

		int _createVoice(int a_note, int a_velocity);

		void _makeIdle(int a_voiceIndex);

		int _findIdleVoice();

		int _getLowestVoiceIndex() const;

		int _getNewestVoiceIndex() const;

		int _getOldestVoiceIndex() const;

		int _getHighestVoiceIndex() const;

	private:
		typedef AtomicQueue<int> VoiceIndexList;
		typedef map<int, AtomicQueue<int> > VoiceMap;

		spsc_queue<ActionMessage*> m_queuedActions;

		unsigned m_numActiveVoices; /// Number of active voices
		unsigned m_maxVoices; /// Total number of voices (idle voices + active voices)
		unsigned m_bufferSize;
		unsigned m_tickCount;
		bool m_isPlaying;

		VoiceMap m_voiceMap;
		VoiceIndexList m_voiceStack;
		VoiceIndexList m_idleVoiceStack;
		VoiceIndexList m_garbageList; /// pre-allocated storage for collecting idle voices during audio processing
		vector<shared_ptr<Circuit> > m_allVoices;
		shared_ptr<Circuit> m_instrument;
		UnitFactory* m_factory;
	};

	template <typename T>
	int VoiceManager::addUnit(T a_prototypeId) {
		shared_ptr<Circuit> voice;
		// Apply action to all voices
		int numVoices = m_allVoices.size();

		shared_ptr<Unit> unit = shared_ptr<Unit>(m_factory->createUnit(a_prototypeId));

		int returnId = m_instrument->addUnit(unit);
		for (int i = 0; i < numVoices; i++) {
			voice = m_allVoices[i];
			returnId = voice->addUnit(shared_ptr<Unit>(unit->clone()));
		}
		return returnId;
	}
}
#endif

