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

/**
 * \file units/MathUnits.h
 * \brief
 * \details
 * \author Austen Satterlee
 * \date 24/01/2016
 */

#ifndef __MATHUNITS__
#define __MATHUNITS__

#include "Unit.h"
#include "units/MemoryUnit.h"
#include "DSPMath.h"

namespace syn {
    const vector<string> scale_selections = {"1","10","1E2","1E3","1E4"};
    const vector<double> scale_values = {1.,10.,100.,1000.,10000.};

    class VOSIMLIB_API MovingAverage {
    public:
        MovingAverage();

        void setWindowSize(int a_newWindowSize);

        double getWindowSize() const;

        double process(double a_input);

        double getPastInputSample(int a_offset);

    private:
        int m_windowSize;
        NSampleDelay m_delay;
        double m_lastOutput;
    };

    /**
     * DC-remover
     */
    class VOSIMLIB_API DCRemoverUnit : public Unit {
        DERIVE_UNIT(DCRemoverUnit)
    public:
        explicit DCRemoverUnit(const string& a_name);

        DCRemoverUnit(const DCRemoverUnit& a_rhs);

        void reset() override;

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;
        void onNoteOn_() override;

    private:
        int m_pAlpha;
        double m_lastOutput;
        double m_lastInput;
    };

    /**
     * Full-wave rectifier
     */
    class VOSIMLIB_API RectifierUnit : public Unit {
        DERIVE_UNIT(RectifierUnit)
    public:
        explicit RectifierUnit(const string& a_name);

        RectifierUnit(const RectifierUnit& a_rhs);

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;

    private:
        int m_pRectType;
    };

    /**
    * Sums incomming signals
    */
    class VOSIMLIB_API SummerUnit : public Unit {
        DERIVE_UNIT(SummerUnit)
    public:
        explicit SummerUnit(const string& a_name);

        SummerUnit(const SummerUnit& a_rhs);

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;
        void onInputConnection_(int a_inputPort) override;
    private:
        int m_pBias;
    };

    /**
     * Multiplies incoming signals
     */
    class VOSIMLIB_API GainUnit : public Unit {
        DERIVE_UNIT(GainUnit)
    public:
        explicit GainUnit(const string& a_name);

        GainUnit(const GainUnit& a_rhs);

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;
        void onInputConnection_(int a_inputPort) override;
    private:
        int m_pGain;
    };

    /**
     * Outputs a constant
     */
    class VOSIMLIB_API ConstantUnit : public Unit {
        DERIVE_UNIT(ConstantUnit)
    public:
        explicit ConstantUnit(const string& a_name);

        ConstantUnit(const ConstantUnit& a_rhs);

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;
    };

    /**
     * Balances incoming signals between two outputs
     */
    class VOSIMLIB_API PanningUnit : public Unit {
        DERIVE_UNIT(PanningUnit)
    public:
        explicit PanningUnit(const string& a_name);

        PanningUnit(const PanningUnit& a_rhs);

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;

    protected:
        int m_pBalance1, m_pBalance2;
    };

    /**
     * Affine transform
     */
    class VOSIMLIB_API LerpUnit : public Unit {
        DERIVE_UNIT(LerpUnit)
    public:
        explicit LerpUnit(const string& a_name);

        LerpUnit(const LerpUnit& a_rhs);

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;

    private:
        int m_pMinInput, m_pMaxInput;
        int m_pMinOutput, m_pMaxOutput;
        int m_pClip;
    };

    /**
    * Pitch (normalized midi note) to frequency conversion
    */
    class VOSIMLIB_API PitchToFreqUnit : public Unit {
        DERIVE_UNIT(PitchToFreqUnit)
    public:
        explicit PitchToFreqUnit(const string& a_name)
            : Unit(a_name) {
            addInput_("in");
            addOutput_("out");
        }

        PitchToFreqUnit(const PitchToFreqUnit& a_rhs)
            :
            PitchToFreqUnit(a_rhs.name()) { }

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override {
            BEGIN_PROC_FUNC
            WRITE_OUTPUT(0, pitchToFreq(READ_INPUT(0)));
            END_PROC_FUNC
        }
    };

    /**
    * Frequency to pitch converter
    */
    class VOSIMLIB_API FreqToPitchUnit : public Unit {
        DERIVE_UNIT(FreqToPitchUnit)
    public:
        explicit FreqToPitchUnit(const string& a_name)
            : Unit(a_name) {
            addInput_("in");
            addOutput_("out");
        }

        FreqToPitchUnit(const FreqToPitchUnit& a_rhs)
            :
            FreqToPitchUnit(a_rhs.name()) { }

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override {
            BEGIN_PROC_FUNC
            WRITE_OUTPUT(0, samplesToPitch(freqToSamples(READ_INPUT(0), fs()), fs()));
            END_PROC_FUNC
        }
    };

    /**
     * Signal switch
     *
     * Output `a` if `ctrl` is larger than `comp`, otherwise output `b`.
     */
    class VOSIMLIB_API SwitchUnit : public Unit {
        DERIVE_UNIT(SwitchUnit)
    public:
        explicit SwitchUnit(const string& a_name)
            : Unit(a_name) {
            addInput_("low");
            addInput_("high");
            addInput_("cmp");
            addInput_("in");
            addOutput_("out");
        }

        SwitchUnit(const SwitchUnit& a_rhs)
            :
            SwitchUnit(a_rhs.name()) { }

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override {
            BEGIN_PROC_FUNC
            double comp = READ_INPUT(2);
            double ctrl = READ_INPUT(3);
            WRITE_OUTPUT(0, ctrl > comp ? READ_INPUT(0) : READ_INPUT(1));
            END_PROC_FUNC
        }
    };

    /**
     * Quantization unit
     */
    class VOSIMLIB_API QuantizerUnit : public Unit {
        DERIVE_UNIT(QuantizerUnit)
    public:
        enum Func {
            Floor = 0,
            Ceil,
            Round
        };

        enum Input {
            iIn = 0,
            iStep,
            NUM_INPUTS
        };

        enum Param {
            pStep = 0,
            NUM_PARAMS
        };

        explicit QuantizerUnit(const string& a_name);

        QuantizerUnit(const QuantizerUnit& a_rhs) : QuantizerUnit(a_rhs.name()) {}

        void reset() override {};

    protected:
        void MSFASTCALL process_() GCCFASTCALL override;
    };

    /**
     * Hyperbolic tangent unit
     */
    class VOSIMLIB_API TanhUnit : public Unit {
        DERIVE_UNIT(TanhUnit)
    public:
        enum Param {
            pSat = 0
        };

        explicit TanhUnit(const string& a_name);
        TanhUnit(const TanhUnit& a_rhs);

        void reset() override {};
    protected:
        void MSFASTCALL process_() GCCFASTCALL override;
    };
}

#endif
