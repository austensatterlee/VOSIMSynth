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
 * \file MathUnits.h
 * \brief
 * \details
 * \author Austen Satterlee
 * \date 24/01/2016
 */

#ifndef __MATHUNITS__
#define __MATHUNITS__

#include "Unit.h"
#include "MemoryUnit.h"
#include "DSPMath.h"

using namespace std;

namespace syn
{
	const vector<string> scale_selections = { "1","10","1E2","1E3","1E4" };
	const vector<double> scale_values = { 1.,10.,100.,1000.,10000. };

	class MovingAverage
	{
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
	class DCRemoverUnit : public Unit
	{
		DERIVE_UNIT(DCRemoverUnit)
	public:
		explicit DCRemoverUnit(const string& a_name);

		DCRemoverUnit(const DCRemoverUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		int m_pAlpha;
		double m_lastOutput;
		double m_lastInput;
	};

	/**
	 * Full-wave rectifier
	 */
	class RectifierUnit : public Unit
	{
		DERIVE_UNIT(RectifierUnit)
	public:
		explicit RectifierUnit(const string& a_name);

		RectifierUnit(const RectifierUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		int m_pRectType;
	};

	/**
	 * Multiplies two signals together
	 */
	class GainUnit : public Unit
	{
		DERIVE_UNIT(GainUnit)
	public:
		explicit GainUnit(const string& a_name);

		GainUnit(const GainUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
	private:
		int m_pGain;
		int m_iGain, m_iInput;
	};

	/**
	* Sums incomming signals
	*/
	class SummerUnit : public Unit
	{
		DERIVE_UNIT(SummerUnit)
	public:
		explicit SummerUnit(const string& a_name);

		SummerUnit(const SummerUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
	};

	/**
	 * Outputs a constant
	 */
	class ConstantUnit : public Unit
	{
		DERIVE_UNIT(ConstantUnit)
	public:
		explicit ConstantUnit(const string& a_name);

		ConstantUnit(const ConstantUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
	};

	/**
	* Balances incoming signals between two outputs
	*/
	class PanningUnit : public Unit
	{
		DERIVE_UNIT(PanningUnit)
	public:
		explicit PanningUnit(const string& a_name);

		PanningUnit(const PanningUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	protected:
		int m_pBalance1, m_pBalance2;
	};

	/**
	* Affine transform
	*/
	class LerpUnit : public Unit
	{
		DERIVE_UNIT(LerpUnit)
	public:
		explicit LerpUnit(const string& a_name);

		LerpUnit(const LerpUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		int m_pMinInput, m_pMaxInput;
		int m_pMinOutput, m_pMaxOutput;
	};

	/**
	* Pitch (normalized midi note) to frequency conversion
	*/
	class PitchToFreqUnit : public Unit
	{
		DERIVE_UNIT(PitchToFreqUnit)
	public:
		explicit PitchToFreqUnit(const string& a_name) : Unit(a_name) 
		{
			addInput_("in");
			addOutput_("out");
		}

		PitchToFreqUnit(const PitchToFreqUnit& a_rhs) : 
			PitchToFreqUnit(a_rhs.getName())
		{
		}

	protected:
		void MSFASTCALL process_() GCCFASTCALL override {
			setOutputChannel_(0,pitchToFreq(getInputValue(0)));
		}
	};


	/**
	* Frequency to pitch (normalized midi note) conversion
	*/
	class FreqToPitchUnit : public Unit
	{
		DERIVE_UNIT(FreqToPitchUnit)
	public:
		explicit FreqToPitchUnit(const string& a_name) : Unit(a_name)
		{
			addInput_("in");
			addOutput_("out");
		}

		FreqToPitchUnit(const FreqToPitchUnit& a_rhs) :
			FreqToPitchUnit(a_rhs.getName())
		{
		}

	protected:
		void MSFASTCALL process_() GCCFASTCALL override {
			setOutputChannel_(0, samplesToPitch(freqToSamples(getInputValue(0),getFs()),getFs()));
		}
	};
}

#endif
