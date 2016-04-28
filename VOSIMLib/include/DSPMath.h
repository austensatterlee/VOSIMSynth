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
#include "NDPoint.h"
#include "tables.h"

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

namespace syn
{
	inline int gcd(int a, int b) {
		while (a != 0) {
			int c = a;
			a = b % a;
			b = c;
		}
		return b;
	}

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

	inline double blackman_harris(int a_k, size_t a_winSize) {
		static const double a[4] = {0.35875, 0.48829, 0.14128, 0.01168};
		double phase = a_k * 1.0 / (a_winSize - 1);
		double s1 = lut_sin.getlinear(phase + 0.25);
		double s2 = lut_sin.getlinear(2 * phase + 0.25);
		double s3 = lut_sin.getlinear(3 * phase + 0.25);
		return a[0] - a[1] * s1 + a[2] * s2 - a[3] * s3;
	}

	inline double pitchToFreq(double pitch) {
		double freq = lut_pitch_table.getlinear(pitch);
		if (freq == 0)
			freq = 1;
		return freq;
	}

	inline double lin2db(double lin, double mindb, double maxdb) {
		double db;
		if (lin >= 0) {
			db = lut_db_table.getlinear(LERP(mindb, maxdb, lin));
		} else {
			db = -lut_db_table.getlinear(LERP(mindb, maxdb, -lin));
		}
		return db;
	}

	template <typename T>
	NDPoint<2, T> closestPointOnLine(const NDPoint<2, T>& pt, const NDPoint<2, T>& a, const NDPoint<2, T>& b) {
		double ablength = a.distFrom(b);
		NDPoint<2, double> abnorm = static_cast<NDPoint<2, double>>(a - b) * (1.0 / ablength);
		double proj = static_cast<NDPoint<2, double>>(pt - b).dot(abnorm);
		proj = CLAMP(proj, 0, ablength);
		return b + abnorm * proj;
	}

	template <typename T>
	double pointLineDistance(const NDPoint<2, T>& pt, const NDPoint<2, T>& a, const NDPoint<2, T>& b) {
		return (pt - closestPointOnLine(pt, a, b)).mag();
	}
}
#endif