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
#include <cmath>

double syn::pitchToFreq(double pitch) {
	return lut_pitch_table().getlinear(pitch);
}

double syn::naive_pitchToFreq(double pitch) {
	return 440.0 * std::pow(2, (pitch - 69) / 12.);
}

double syn::bpmToFreq(double bpm, double tempo)
{
	return bpm > 1e-5 ? tempo / (60.0 * bpm) : 0.0;
}

double syn::freqToSamples(double freq, double fs)
{
	return freq > 1e-5 ? fs/freq : 0.0;
}

double syn::periodToSamples(double seconds, double fs)
{
	return seconds*fs;
}

double syn::samplesToPitch(double samples, double fs) {
	return samples > 1e-5 ? log2(fs/(440*samples))*12. + 69 : 0.0;
}

double syn::samplesToFreq(double samples, double fs) {
	return samples > 1e-5 ? fs / samples : 0.0;
}

double syn::samplesToPeriod(double samples, double fs) {
	return samples / fs;
}

double syn::samplesToBPM(double samples, double fs, double tempo) {
	return samples > 1e-5 ? tempo * samples / (60.0 * fs) : 0.0;
}

double syn::lin2db(double lin) {
	return 20 * log10(MAX<double>(lin, 0));
}

double syn::db2lin(double db) {
	return pow(10, db / 20.);
}

double syn::blackman_harris(int a_k, size_t a_winSize) {
	static const double a[4] = { 0.35875, 0.48829, 0.14128, 0.01168 };
	double phase = a_k * 1.0 / (a_winSize - 1);
	double s1 = lut_sin_table().getlinear_periodic(phase + 0.25);
	double s2 = lut_sin_table().getlinear_periodic(2 * phase + 0.25);
	double s3 = lut_sin_table().getlinear_periodic(3 * phase + 0.25);
	return a[0] - a[1] * s1 + a[2] * s2 - a[3] * s3;
}

double syn::hanning(int a_k, size_t a_winSize) {
	return 0.5*(1 - lut_sin_table().getlinear_periodic(a_k * 1.0 / (a_winSize - 1)));
}

double syn::naive_tri(double a_phase) {
	a_phase = WRAP(a_phase, 1.0);
	return a_phase <= 0.5 ? 4 * a_phase - 1 : -4 * (a_phase - 0.5) + 1;
}

double syn::naive_saw(double a_phase) {
	a_phase = WRAP(a_phase, 1.0);
	return a_phase < 0.5 ? 2 * a_phase : 2 * a_phase - 2;
}

double syn::naive_square(double a_phase) {
	a_phase = WRAP(a_phase, 1.0);
	return a_phase <= 0.5 ? 1 : -1;
}

std::string syn::incrementSuffix(const std::string& a_str) {
	std::regex rexp("^(.*?[^\\d])(\\d+)$");
	std::smatch match;
	regex_match(a_str, match, rexp);
	if (!match[2].matched) {
		// numbered suffix not found	
		std::string newstring = a_str + "_0";
		return newstring;
	} else {
		int suffix_num = stoi(match[2].str());
		std::string new_name = match[1].str() + std::to_string(suffix_num + 1);
		return new_name;
	}
}
