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
* \file Filter.h
* \brief
* \details
* \author Austen Satterlee
* \date March 6, 2016
*/
#ifndef __FILTER__
#define __FILTER__
#include "Unit.h"

namespace syn
{
	template <size_t nX, size_t nY>
	class VOSIMLIB_API Filter : public Unit
	{
	protected:
		double XCoefs[nX];
		double YCoefs[nY];
		double XBuf[nX];
		double YBuf[nY];
		int xBufInd, yBufInd;
	public:
		Filter(string a_name, const double a_X[nX], const double a_Y[nY]) :
			Unit(a_name),
			xBufInd(0),
			yBufInd(0) {
			addInput_("in");
			addOutput_("out");
			memcpy(XCoefs, a_X, sizeof(double) * nX);
			memcpy(YCoefs, a_Y, sizeof(double) * nY);
			reset_();
		};

		Filter(const Filter &a_rhs) :
			Filter(a_rhs.name(), XCoefs, YCoefs) { }

		virtual ~Filter() { };

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

		void reset_() {
			memset(YBuf, 0, nY * sizeof(double));
			memset(XBuf, 0, nX * sizeof(double));
			xBufInd = 0;
			yBufInd = 0;
		};

	private:
		string _getClassName() const override {
			return "Filter";
		};
	};

	template <size_t nX, size_t nY>
	void Filter<nX, nY>::process_() {
		XBuf[xBufInd] = readInput(0);
		YBuf[yBufInd] = 0.0;
		double *output = &YBuf[yBufInd];
		int i, j;
		for (i = 0, j = xBufInd; i < nX; i++, j--) {
			if (j < 0)
				j = nX - 1;
			YBuf[yBufInd] += XBuf[j] * XCoefs[i];
		}
		for (i = 1, j = yBufInd - 1; i < nY; i++, j--) {
			if (j < 0)
				j = nY - 1;
			YBuf[yBufInd] -= YBuf[j] * YCoefs[i];
		}
		xBufInd++;
		if (xBufInd == nX)
			xBufInd = 0;
		yBufInd++;
		if (yBufInd == nY)
			yBufInd = 0;
		setOutputChannel_(0, *output);
	}
}
#endif
