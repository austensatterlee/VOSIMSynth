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
#include "CircularContainers.h"
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/policies.hpp>
#include <Containers.h>
#include <map>

#define MAX_VOICEMANAGER_MSG_QUEUE_SIZE 1024
#define MAX_VOICES 16

using std::map;
using std::string;
using boost::lockfree::spsc_queue;
using boost::lockfree::capacity;

namespace syn
{
    /**
     * \brief Used to pass messages to a voice manager via VoiceManager::queueAction.
     *
     * The action function pointer will be called once for each voice (with the corresponding circuit passed as the first parameter).
     * On the last voice, the second parameter will be true.
     * The third parameter is a pointer to the ByteChunk stored in this structure, which is useful for passing arguments.
     */
    struct VOSIMLIB_API RTMessage
    {
        void (*action)(Circuit*, bool, ByteChunk*);
        ByteChunk data;
    };

    class VOSIMLIB_API VoiceManager
    {
    public:
        enum VoiceSelectionPolicy
        {
            LOW = 0,
            HIGH,
            OLD
        };

    public:
        VoiceManager() :
            m_queuedActions{MAX_VOICEMANAGER_MSG_QUEUE_SIZE},
            m_numActiveVoices(0),
            m_maxVoices(0),
            m_bufferSize(1),
            m_internalBufferSize(1),
            m_tickCount(0),
            m_activeVoices(0),
            m_idleVoices(0),
            m_garbageList(0),
            m_instrument{"main"}
        {
            setBufferSize(m_bufferSize);
            setInternalBufferSize(m_internalBufferSize);
        };

        virtual ~VoiceManager() { }

        void MSFASTCALL tick(const double* a_left_input, const double* a_right_input, double* a_left_output, double* a_right_output) GCCFASTCALL;

        /**
         * Safely queue a function to be called on the real-time thread in between samples.
         */
        unsigned queueAction(RTMessage* a_action);

        unsigned getTickCount() const;

        void setFs(double a_newFs);

        /**
         * \brief The number of samples read and produced by the tick() method of the VoiceManager.
         */
        void setBufferSize(int a_bufferSize);

        /**
         * \brief Set the number of samples read and produced by the tick() method of the internal circuits.
         */
        void setInternalBufferSize(int a_internalBufferSize);
        int getInternalBufferSize() const { return m_internalBufferSize; }

        void setTempo(double a_newTempo);

        void noteOn(int a_noteNumber, int a_velocity);

        void noteOff(int a_noteNumber, int a_velocity);

        void sendControlChange(int a_cc, double a_newvalue);

        void setMaxVoices(unsigned a_newMax);

        vector<int> getActiveVoiceIndices() const;

        int getMaxVoices() const;

        int getLowestVoiceIndex() const;
        int getNewestVoiceIndex() const;
        int getOldestVoiceIndex() const;
        int getHighestVoiceIndex() const;

        void onIdle();

        /**
         * Retrieves a unit from a specific voice circuit.
         * If the given voice id is negative, the unit is retrieved from the prototype circuit.
         */
        Unit& getUnit(int a_id, int a_voiceId = -1);
        const Unit& getUnit(int a_id, int a_voiceId = -1) const;

        Circuit* getPrototypeCircuit();
        const Circuit* getPrototypeCircuit() const;

        Circuit* getVoiceCircuit(int a_voiceId);
        const Circuit* getVoiceCircuit(int a_voiceId) const;

        void setPrototypeCircuit(const Circuit& a_circ);

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

        int _stealIdleVoice();

    private:
        typedef CircularQueue<int> VoiceIndexList;
        typedef map<int, CircularQueue<int>> VoiceMap;

        spsc_queue<RTMessage*> m_queuedActions;

        vector<Circuit> m_circuits;
        int m_numActiveVoices; /// Number of active voices
        int m_maxVoices; /// Total number of usable voices (idle voices + active voices)
        int m_bufferSize;
        int m_internalBufferSize;
        int m_tickCount;

        VoiceMap m_voiceMap; /// maps midi notes to voice indices
        VoiceIndexList m_activeVoices; /// list of active voice indices
        VoiceIndexList m_idleVoices; /// list of idle voice indices
        VoiceIndexList m_garbageList; /// pre-allocated storage for collecting idle voices during audio processing
        Circuit m_instrument;
    };
}
#endif
