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

#include "units/Follower.h"

syn::FollowerUnit::FollowerUnit(const string& a_name) :
    Unit(a_name),
    m_w(0.0),
    m_output(0.0),
    m_pAlpha(addParameter_(UnitParameter("hp", 0.0, 1.0, 0.995))),
    m_pBeta(addParameter_(UnitParameter("lp", 0.0, 1.0, 0.01)))
{
    addInput_("in");
    addOutput_("out");
}

syn::FollowerUnit::FollowerUnit(const FollowerUnit& a_rhs) :
    FollowerUnit(a_rhs.name()) {}

void syn::FollowerUnit::reset()
{
    m_w = 0.0;
    m_output = 0.0;
}

void syn::FollowerUnit::process_()
{
    BEGIN_PROC_FUNC
        double alpha = param(m_pAlpha).getDouble();
        double beta = param(m_pBeta).getDouble();
        double input = READ_INPUT(0) * 0.5 * (1 + alpha);
        // dc removal + rectification
        double old_w = m_w;
        m_w = input + alpha * old_w;
        double hpOutput = abs(m_w - old_w);
        // low pass
        m_output = beta * hpOutput + (1 - beta) * m_output;
        WRITE_OUTPUT(0, sqrt(m_output));
    END_PROC_FUNC
}

void syn::FollowerUnit::onNoteOn_()
{
    reset();
}
