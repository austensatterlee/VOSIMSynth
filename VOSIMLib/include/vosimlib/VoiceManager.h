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
#include "vosimlib/Circuit.h"
#include "vosimlib/Unit.h"
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/policies.hpp>

#define MAX_VOICEMANAGER_MSG_QUEUE_SIZE 1024
#define MAX_VOICES 16

using std::string;
using boost::lockfree::spsc_queue;
using boost::lockfree::capacity;

namespace syn {
    
    class Command;

    class VOSIMLIB_API VoiceManager {
    public:
        enum VoiceStealPolicy {
            Oldest = 0,
            Newest,
            Lowest,
            Highest
        };

    public:
        VoiceManager()
            :
            m_queuedActions{MAX_VOICEMANAGER_MSG_QUEUE_SIZE},
            m_lastVoiceIndex(0),
            m_voiceTicks(0),
            m_bufferSize(1),
            m_internalBufferSize(1),
            m_instrument{"main"},
            m_voiceStealingPolicy(Oldest),
            m_legato(false)
        {
            setBufferSize(m_bufferSize);
            setInternalBufferSize(m_internalBufferSize);
        }

        void tick(const double* a_left_input, const double* a_right_input, double* a_left_output, double* a_right_output) ;

        /**
         * Safely queue a function to be called on the real-time thread in between samples.
         * 
         * Note that this function should ONLY be called from the gui thread! This is a single-producer
         * single-consumer queue!
         * 
         * \returns True if the message was pushed onto the queue, false if the queue was full.
         */
        bool queueAction(Command* a_action);

        /**
         * \brief The number of samples read and produced by the tick() method of the VoiceManager.
         */
        void setBufferSize(int a_bufferSize);

        /**
         * \brief Set the number of samples read and produced by the tick() method of the internal circuits.
         */
        void setInternalBufferSize(int a_internalBufferSize);
        int getInternalBufferSize() const { return m_internalBufferSize; }

        void setFs(double a_newFs);
        void setTempo(double a_newTempo);
        void noteOn(int a_noteNumber, int a_velocity);
        void noteOff(int a_noteNumber);
        void sendControlChange(int a_cc, double a_value);
        void sendPitchWheelChange(double a_value);

        void setMaxVoices(int a_newMax);

        vector<int> getActiveVoiceIndices() const;
        vector<int> getReleasedVoiceIndices() const;
        vector<int> getIdleVoiceIndices() const;

        int getMaxVoices() const;

        int getNewestVoiceIndex() const;

        void onIdle();

        /**
         * Retrieves a unit from a specific voice circuit.
         * If \p a_voiceId is negative, the unit is retrieved from the prototype circuit.
         */
        Unit& getUnit(int a_id, int a_voiceInd = -1);
        const Unit& getUnit(int a_id, int a_voiceInd = -1) const;

        Circuit& getPrototypeCircuit();
        const Circuit& getPrototypeCircuit() const;

        /**
         * Retrieve the specified voice circuit. Returns the prototype circuit if \p a_voiceId is negative.
         */
        Circuit& getVoiceCircuit(int a_voiceId);
        const Circuit& getVoiceCircuit(int a_voiceId) const;

        void setPrototypeCircuit(const Circuit& a_circ);

        VoiceStealPolicy getVoiceStealPolicy() const { return m_voiceStealingPolicy; }
        void setVoiceStealPolicy(VoiceStealPolicy a_newPolicy) { m_voiceStealingPolicy = a_newPolicy; }

    private:
        /**
         * Processes all actions from the action queue
         */
        void _flushActionQueue();

    private:
        spsc_queue<Command*> m_queuedActions;

        vector<Circuit> m_voices;
        vector<int> m_voiceBirths; ///< value of `m_voiceTicks` recorded upon voice activation
        int m_lastVoiceIndex;
        int m_voiceTicks; ///< counts the total number of voices activated since the beginning
        int m_bufferSize; ///< size of the buffers that will be written to by VoiceManager::tick
        int m_internalBufferSize; ///< size of the voice buffers that will be read from by VoiceManager::tick

        Circuit m_instrument;

        VoiceStealPolicy m_voiceStealingPolicy; ///< Determines which voices are replaced when all of them are active

        bool m_legato; ///< When true, voices get reset upon activation only if they are in the "note off" state.
    };
}
#endif
