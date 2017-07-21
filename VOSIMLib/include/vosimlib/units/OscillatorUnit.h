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

#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include "vosimlib/Unit.h"

namespace syn
{
    enum WAVE_SHAPE
    {
        SAW_WAVE = 0,
        SINE_WAVE,
        TRI_WAVE,
        SQUARE_WAVE
    };

    const vector<string> WAVE_SHAPE_NAMES{ "Saw", "Sine", "Tri", "Square" };

    class VOSIMLIB_API OscillatorUnit : public Unit
    {
    public:
        enum Param
        {
            pGain = 0,
            pPhaseOffset,
            pUnipolar,
            NUM_PARAMS
        };

        enum Output
        {
            oOut = 0,
            oPhase,
            NUM_OUTPUTS
        };

        enum Input
        {
            iGainMul = 0,
            iPhaseAdd,
            iSync,
            NUM_INPUTS
        };

        explicit OscillatorUnit(const string& a_name);

        void reset() override;

    protected:
        void process_() override;
        virtual void tickPhase_(double a_phaseOffset) ;
        virtual void updatePhaseStep_() ;

    protected:
        double m_basePhase;
        double m_phase, m_last_phase;
        double m_phase_step;
        double m_period;
        double m_freq;
        double m_gain;
        double m_bias;
        double m_lastSync;
    };

    class VOSIMLIB_API TunedOscillatorUnit : public OscillatorUnit
    {
    public:
        enum Param
        {
            pTune = OscillatorUnit::NUM_PARAMS,
            pOctave,
            NUM_PARAMS
        };

        enum Input
        {
            iNote = OscillatorUnit::NUM_INPUTS,
            NUM_INPUTS
        };

        explicit TunedOscillatorUnit(const string& a_name);

        explicit TunedOscillatorUnit(const TunedOscillatorUnit& a_rhs);

    protected:
        void process_() override ;
        void updatePhaseStep_() override ;
        void onNoteOn_() override;
    protected:
        double m_pitch;
    };

    class VOSIMLIB_API BasicOscillatorUnit : public TunedOscillatorUnit
    {
        DERIVE_UNIT(BasicOscillatorUnit)
    public:
        enum Param
        {
            pWaveform = TunedOscillatorUnit::NUM_PARAMS,
            NUM_PARAMS
        };
        explicit BasicOscillatorUnit(const string& a_name);

        explicit BasicOscillatorUnit(const BasicOscillatorUnit& a_rhs);

    protected:
        void process_() override;
    };

    class VOSIMLIB_API LFOOscillatorUnit : public OscillatorUnit
    {
        DERIVE_UNIT(LFOOscillatorUnit)

    public:
        enum Param
        {
            pWaveform = OscillatorUnit::NUM_PARAMS,
            pBPMFreq,
            pFreq,
            pTempoSync,
            NUM_PARAMS
        };

        enum Input
        {
            iFreqAdd = OscillatorUnit::NUM_INPUTS,
            iFreqMul,
            NUM_INPUTS
        };

        enum Output
        {
            oQuadOut = OscillatorUnit::NUM_OUTPUTS,
            NUM_OUTPUTS
        };

        explicit LFOOscillatorUnit(const string& a_name);

        explicit LFOOscillatorUnit(const LFOOscillatorUnit& a_rhs) : LFOOscillatorUnit(a_rhs.name()) {}

    protected:
        void process_() override;
        void onParamChange_(int a_paramId) override;
    };
}
#endif
