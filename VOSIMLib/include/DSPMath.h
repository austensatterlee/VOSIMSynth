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

#define DSP_PI 3.14159265359f

namespace syn
{
	inline int gcd(int a, int b);

	template <typename T>
	T LERP(const T& pt1, const T& pt2, const T& frac) {
		return (pt2 - pt1)*frac + pt1;
	}

	template <typename T>
	T INVLERP(const T& pt1, const T& pt2, const T& lerped_pt) {
		return (lerped_pt - pt1) / (pt2 - pt1);
	}

	template <typename T>
	T CLAMP(const T& val, const T& min_val, const T& max_val) {
		return val < min_val ? min_val : (val > max_val ? max_val : val);
	}

	template <typename T>
	T MAX(const T& val1, const T& val2) {
		return val1 < val2 ? val2 : val1;
	}

	template <typename T>
	T MIN(const T& val1, const T& val2) {
		return val1 > val2 ? val2 : val1;
	}

	template <typename T>
	T WRAP(const T& x, const T& m) {
		if (!m)
			return x;
		T newx = x;
		while (newx >= m)
			newx -= m;
		while (newx < 0)
			newx += m;
		return newx;
	}

	template <typename T>
	T WRAP2(const T& x, const T& left_m, const T& right_m) {
		const T m = right_m - left_m;
		if (!m)
			return x;
		T newx = x;
		while (newx >= right_m)
			newx -= m;
		while (newx < left_m)
			newx += m;
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
