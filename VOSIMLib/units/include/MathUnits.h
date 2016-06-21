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
	 * Bilinear/trapezoidal integrator
	 */
	class BLIntegrator
	{
	public:
		BLIntegrator();

		/**
		 * Set normalized cutoff frequency in the range (0,1), where 1 is nyquist.
		 */
		void setFc(double a_newfc);

		double process(double a_input);

	private:
		double m_state;
		double m_normfc; /// normalized cutoff frequency
	};

	/**
	 * DC-remover
	 */
	class DCRemoverUnit : public Unit
	{
	public:
		explicit DCRemoverUnit(const string& a_name);

		DCRemoverUnit(const DCRemoverUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override;

		Unit* _clone() const override;

	private:
		int m_pAlpha;
		double m_lastOutput;
		double m_lastInput;
	};

	/**
	* 1 Pole Filter (Lag)
	*/
	class LagUnit : public Unit
	{
	public:
		explicit LagUnit(const string& a_name);

		LagUnit(const LagUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override;

		Unit* _clone() const override;

	private:
		int m_pFc;
		int m_iFcAdd, m_iFcMul;
		double m_state;
	};

	/**
	 * Full-wave rectifier
	 */
	class RectifierUnit : public Unit
	{
	public:
		explicit RectifierUnit(const string& a_name);

		RectifierUnit(const RectifierUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override;

		Unit* _clone() const override;

	private:
		int m_pRectType;
	};

	/**
	 * Performs multiplies two signals and then adds a third signal as a bias
	 */
	class MACUnit : public Unit
	{
	public:
		explicit MACUnit(const string& a_name);

		MACUnit(const MACUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override;

		Unit* _clone() const override;
	};

	/**
	 * Applies gain to the difference between the two inputs (like an op amp)
	 */
	class GainUnit : public Unit
	{
	public:
		explicit GainUnit(const string& a_name);

		GainUnit(const GainUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override;

		Unit* _clone() const override;

	private:
		int m_pGain, m_pScale;
		int m_iGain, m_iInput, m_iInvInput;
	};

	/**
	* Sums incomming signals
	*/
	class SummerUnit : public Unit
	{
	public:
		explicit SummerUnit(const string& a_name);

		SummerUnit(const SummerUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override;

		Unit* _clone() const override;
	};

	/**
	 * Outputs a constant
	 */
	class ConstantUnit : public Unit
	{
	public:
		explicit ConstantUnit(const string& a_name);

		ConstantUnit(const ConstantUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override;

		Unit* _clone() const override;
	};

	/**
	* Balances incoming signals between two outputs
	*/
	class PanningUnit : public Unit
	{
	public:
		explicit PanningUnit(const string& a_name);

		PanningUnit(const PanningUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	protected:
		int m_pBalance1, m_pBalance2;
	private:
		string _getClassName() const override;

		Unit* _clone() const override;
	};

	/**
	* Affine transform
	*/
	class LerpUnit : public Unit
	{
	public:
		explicit LerpUnit(const string& a_name);

		LerpUnit(const LerpUnit& a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override;

		Unit* _clone() const override;

	private:
		int m_pInputRange;
		int m_pMinOutput, m_pMaxOutput;
		int m_pMinOutputScale, m_pMaxOutputScale;
	};

	/**
	* Affine transform
	*/
	class PitchToFreqUnit : public Unit
	{
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

	private:
		string _getClassName() const override { return "PitchToFreqUnit"; }

		Unit* _clone() const override { return new PitchToFreqUnit(*this); }
	};
}

#endif
