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
* \file ADSREnvelope.h
* \brief
* \details
* \author Austen Satterlee
* \date March 6, 2016
*/

#ifndef __ADSRENVELOPE__
#define __ADSRENVELOPE__
#include "Unit.h"

namespace syn
{
	class VOSIMLIB_API ADSREnvelope : public Unit
	{
		DERIVE_UNIT(ADSREnvelope)
	public:
		explicit ADSREnvelope(const string &name);

		ADSREnvelope(const ADSREnvelope &a_rhs);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

	public:
		virtual bool isActive() const override;

		void trigger();
		void release();
	private:
		enum EADSRStage
		{
			Attack = 0,
			Decay,
			Sustain,
			Release
		};

		double m_phase;
		EADSRStage m_currStage;
		double m_initial;
		double m_target;
		bool m_isActive;
		int m_pAttack, m_pDecay, m_pSustain, m_pRelease, m_pTimeScale;
		int m_iAttack, m_iDecay, m_iRelease;
		int m_iAttackMul, m_iDecayMul, m_iReleaseMul;
		int m_iGate;
	};
}
#endif
