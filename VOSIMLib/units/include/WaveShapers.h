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
	class InvTanUnit : public Unit
	{
		DERIVE_UNIT(InvTanUnit)
	public:
		explicit InvTanUnit(const string& a_name) : Unit(a_name)
		{
			addInput_("in");
			addOutput_("out");
		}
		InvTanUnit(const InvTanUnit& a_rhs) : InvTanUnit(a_rhs.getName()) {}
	protected:
		void MSFASTCALL process_() GCCFASTCALL override {
			double input = getInputValue(0);
			setOutputChannel_(0, atan(input) / DSP_PI*0.5);
		}
	};
}

CEREAL_REGISTER_TYPE(syn::InvTanUnit);

#endif
