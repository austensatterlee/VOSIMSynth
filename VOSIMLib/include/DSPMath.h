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
* \file DSPMath.h
* \brief
* \details
* \author Austen Satterlee
* \date March 6, 2016
*/

#ifndef __DSPMATH__
#define __DSPMATH__

#if defined(_MSC_VER)
#define MSFASTCALL __fastcall
#define GCCFASTCALL 
#elif defined(__GNUC__)
#define MSFASTCALL
#define GCCFASTCALL __attribute__((fastcall))
#endif

#define LERP(A,B,F) (((B)-(A))*(F)+(A))
#define INVLERP(A,B,X) (((X)-(A))/((B)-(A)))
#define CLAMP(x,lo,hi) ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define DSP_PI 3.14159265359f

namespace syn
{
	inline int gcd(int a, int b);

	template <typename T>
	T WRAP(T x, T modulo) {
		if (!modulo)
			return x;
		T newx = x;
		while (newx >= modulo) {
			newx -= modulo;
		}
		while (newx < 0) {
			newx += modulo;
		}
		return newx;
	}

	double blackman_harris(int a_k, size_t a_winSize);

	double pitchToFreq(double pitch);

	double bpmToFreq(double bpm, double tempo);

	double freqToSamples(double freq, double fs);

	double periodToSamples(double seconds, double fs);

	double lin2db(double lin, double mindb, double maxdb);
}
#endif
