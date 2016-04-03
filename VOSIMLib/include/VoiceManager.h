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
#include <map>
#include <memory>

#define MAX_QUEUE_SIZE 64
#define MAX_VOICES_PER_NOTE 8

using std::map;
using std::string;
using std::shared_ptr;

namespace syn
{
	/* Modifying actions that should be queued and processed in between samples */
	enum EActionType
	{
		ModifyParam,
		ModifyParamNorm,
		ModifyParamPrecision,
		CreateUnit,
		DeleteUnit,
		ConnectInput,
		ConnectOutput,
		ConnectInternal,
		DisconnectInput,
		DisconnectOutput,
		DisconnectInternal
	};

	struct ActionArgs
	{
		int id1;
		int id2;

		union
		{
			int id3;
			double value;
		};

		int id4;
	};

	struct ActionMessage
	{
		EActionType action;
		ActionArgs args;
	};

	class VoiceManager
	{
	public:
		VoiceManager(shared_ptr<Circuit> a_proto, shared_ptr<UnitFactory> a_factory) :
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

		/**
		 * Queue an action to be processed before the next sample.
		 *
		 * The following is a description of parameter signatures corresponding to each possible action:
		 * ModifyParam:				(int unit_id, int param_id, double value)
		 * ModifyParamNorm:			(int unit_id, int param_id, double norm_value)
		 * ModifyParamPrecision:	(int unit_id, int param_id, int new_precision)
		 * DeleteUnit:				(int unit_id)
		 * ConnectInput:			(int circuit_input_id, int unit_id, int unit_input_id)
		 * ConnectOutput:			(int circuit_output_id, int unit_id, int output_port_id)
		 * ConnectInternal:			(int from_unit_id, int from_unit_port, int to_unit_id, int to_unit_port)
		 * DisconnectInput:			(int circuit_input_id, int unit_id, int input_port_id)
		 * DisconnectOutput:		(int circuit_output_id, int unit_id, int output_port_id)
		 * DisconnectInternal:		(int from_unit_id, int from_unit_port, int to_unit_id, int to_unit_port
		 */
		unsigned queueAction(EActionType a_action, const ActionArgs& a_params);

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
		void _processAction(const ActionMessage& a_msg);

		int _createVoice(int a_note, int a_velocity);

		void _makeIdle(int a_voiceIndex);

		int _findIdleVoice();

		int _getLowestVoiceIndex() const;

		int _getNewestVoiceIndex() const;

		int _getOldestVoiceIndex() const;

		int _getHighestVoiceIndex() const;

	private:
		typedef AtomicQueue<int> VoiceList;
		typedef map<int, VoiceList> VoiceMap;

		AtomicQueue<ActionMessage> m_queuedActions;

		unsigned m_numActiveVoices; /// Number of active voices
		unsigned m_maxVoices; /// Total number of voices (idle voices + active voices)
		unsigned m_bufferSize;
		unsigned m_tickCount;
		bool m_isPlaying;

		VoiceMap m_voiceMap;
		VoiceList m_voiceStack;
		VoiceList m_idleVoiceStack;
		VoiceList m_garbageList; /// pre-allocated storage for collecting idle voices during audio processing
		vector<shared_ptr<Circuit>> m_allVoices;
		shared_ptr<Circuit> m_instrument;
		shared_ptr<UnitFactory> m_factory;
	};

	template <typename T>
	int VoiceManager::addUnit(T a_prototypeId) {
		shared_ptr<Circuit> voice;
		// Apply action to all voices
		int numVoices = m_allVoices.size();

		shared_ptr<Unit> unit = m_factory->createUnit(a_prototypeId);

		int returnId = m_instrument->addUnit(unit);
		for (int i = 0; i < numVoices; i++) {
			voice = m_allVoices[i];
			returnId = voice->addUnit(shared_ptr<Unit>(unit->clone()));
		}
		return returnId;
	}
}
#endif

