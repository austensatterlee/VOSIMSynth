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

#pragma once
#include "vosimlib/Unit.h"

namespace syn {
    /**
     * General N-Sample delay
     */
    class VOSIMLIB_API NSampleDelay {
    public:
        NSampleDelay();

        /**
         * \brief Return the most recent output calculated by process().
         * \return 
         */
        double getLastOutput() const;
        /**
         * \brief Record a sample to the buffer and return the delay output.
         * \param a_input 
         * \return 
         */
        double process(double a_input);
        /**
         * \brief Record a sample to the buffer and increments the write location.
         * \param a_input 
         */
        void silentProcess(double a_input);
        /**
         * \brief Read from the buffer at an arbitray delay time
         * \param a_offset delay time (in samples)
         * \return 
         */
        double readTap(double a_offset);
        void resizeBuffer(double a_newBufSize);
        void reset();
        int size() const;
    private:
        vector<double> m_buffer;
        int m_arraySize;
        double m_delaySamples;
        int m_curWritePhase;
        double m_lastOutput;
    };

    /**
     * Single sample delay unit
     */
    class VOSIMLIB_API MemoryUnit : public Unit {
        DERIVE_UNIT(MemoryUnit)
    public:
        explicit MemoryUnit(const string& a_name);
        MemoryUnit(const MemoryUnit& a_rhs);

        virtual void reset() override;

    protected:
        void onParamChange_(int a_paramId) override;
        void process_() override;
        void onNoteOn_() override;

    private:
        NSampleDelay m_delay;
    };

    /**
     * Delay with variable-sized buffer and support for multiple units of time.
     */
    class VOSIMLIB_API VariableMemoryUnit : public Unit {
        DERIVE_UNIT(VariableMemoryUnit)

    public:
        enum Param {
            pBufDelay = 0,
            pBufFreq,
            pBufBPMFreq,
            pBufSamples,
            pBufType,
            pDryGain,
            pWetGain
        };

        enum Output {
            oOut = 0,
            oSend
        };

        enum Input {
            iIn = 0,
            iReceive,
            iSizeMod
        };

        explicit VariableMemoryUnit(const string& a_name);
        VariableMemoryUnit(const VariableMemoryUnit& a_rhs);

        void reset() override;

    protected:
        void onParamChange_(int a_paramId) override;
        void process_() override;
        void onNoteOn_() override;
    private:
        NSampleDelay m_delay;
        double m_delaySamples;
        double m_lastOutput;
    };

    /**
     * Sample & Hold unit
     * 
     * User can set the frequency using the frequency parameter. 
     * When the sync input falls (falling edge), the phase resets and a new sample is read.
     * If the frequency parameter is set to 0.0, the sample and hold
     * can be completely controlled with the sync input.
     */
    class VOSIMLIB_API SampleAndHoldUnit : public Unit {
        DERIVE_UNIT(SampleAndHoldUnit)
    public:
        enum Param {
            pFreq = 0
        };

        enum Input {
            iIn = 0,
            iSync
        };

        explicit SampleAndHoldUnit(const string& a_name);

        SampleAndHoldUnit(const SampleAndHoldUnit& a_rhs);

        void reset() override;;
    protected:
        void process_() override;

        void onParamChange_(int a_paramId) override;
    private:
        double m_currValue;
        double m_phase;
        double m_step;
        double m_lastSync;
    };
}
