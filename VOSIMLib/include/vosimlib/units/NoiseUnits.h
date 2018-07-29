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
#include "vosimlib/DSPMath.h"

namespace syn {
    /**
     * Noise generator
     */
    class VOSIMLIB_API NoiseUnit : public syn::Unit {
    DERIVE_UNIT(NoiseUnit)
    public:
        enum Input {
            iGainMul = 0
        };

        explicit NoiseUnit(const string& a_name)
            : Unit(a_name) {
            addInput_(iGainMul, "gain", 1.0);
            addOutput_("out");
        };

        NoiseUnit(const NoiseUnit& a_rhs)
            : NoiseUnit(a_rhs.name()) {};

        void reset() override {};
    protected:
        void process_() override {
            BEGIN_PROC_FUNC
            double out = 2. * (rand() + 1.) / (RAND_MAX + 2.) - 1.;
            double gain = READ_INPUT(iGainMul);
            out = gain * out;
            WRITE_OUTPUT(0, out);
            END_PROC_FUNC
        };
    };
}
