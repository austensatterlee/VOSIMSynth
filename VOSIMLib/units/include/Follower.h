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
* \file Follower.h
* \brief
* \details
* \author Austen Satterlee
*/

#ifndef __FOLLOWER__
#define __FOLLOWER__

#include "Unit.h"

namespace syn
{
	/**
	* Envelope follower
	*/
	class VOSIMLIB_API FollowerUnit : public Unit
	{
		DERIVE_UNIT(FollowerUnit)
	public:
		explicit FollowerUnit(const string &a_name);

		FollowerUnit(const FollowerUnit &a_rhs);
		void reset() override;
	protected:
		void MSFASTCALL process_() GCCFASTCALL override;
	private:
		double m_w;
		double m_output;

		int m_pAlpha;
		int m_pBeta;
	};
};
#endif
