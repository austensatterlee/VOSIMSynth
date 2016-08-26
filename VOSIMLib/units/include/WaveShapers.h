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
 *  \file WaveShapers.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 07/2016
 */

#ifndef __WAVESHAPERS__
#define __WAVESHAPERS__
#include <Unit.h>
#include <DSPMath.h>

namespace syn {
	class TanhUnit : public Unit
	{
		DERIVE_UNIT(TanhUnit)
	public:
		enum Param
		{
			pSat = 0
		};
		explicit TanhUnit(const string& a_name) : Unit(a_name)
		{
			addParameter_(pSat, UnitParameter("sat", 1.0, 10.0, 1.0));
			addInput_("in");
			addOutput_("out");
		}
		TanhUnit(const TanhUnit& a_rhs) : TanhUnit(a_rhs.getName()) {}
	protected:
		void MSFASTCALL process_() GCCFASTCALL override {
			double input = getInputValue(0);
			double sat = getParameter(pSat).getDouble();
			setOutputChannel_(0, fast_tanh_poly(input*sat)/fast_tanh_poly(sat));
		}
	};
}

CEREAL_REGISTER_TYPE(syn::TanhUnit);

#endif
