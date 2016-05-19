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
* \file DSPMath.cpp
* \brief
* \details
* \author Austen Satterlee
* \date March 6, 2016
*/

#include "DSPMath.h"
#include "tables.h"

int syn::gcd(int a, int b) {
	while (a != 0) {
		int c = a;
		a = b % a;
		b = c;
	}
	return b;
}

double syn::blackman_harris(int a_k, size_t a_winSize) {
	static const double a[4] = {0.35875, 0.48829, 0.14128, 0.01168};
	double phase = a_k * 1.0 / (a_winSize - 1);
	double s1 = lut_sin.getlinear(phase + 0.25);
	double s2 = lut_sin.getlinear(2 * phase + 0.25);
	double s3 = lut_sin.getlinear(3 * phase + 0.25);
	return a[0] - a[1] * s1 + a[2] * s2 - a[3] * s3;
}

double syn::pitchToFreq(double pitch) {
	double freq = lut_pitch_table.getlinear(pitch);
	return freq;
}

double syn::bpmToFreq(double bpm, double tempo)
{
	return tempo / 60.0 * 1. / bpm;
}

double syn::freqToSamples(double freq, double fs)
{
	return fs/freq;
}

double syn::periodToSamples(double seconds, double fs)
{
	return seconds*fs;
}

double syn::lin2db(double lin, double mindb, double maxdb) {
	double db;
	if (lin >= 0) {
		db = lut_db_table.getlinear(LERP(mindb, maxdb, lin));
	} else {
		db = -lut_db_table.getlinear(LERP(mindb, maxdb, -lin));
	}
	return db;
}
