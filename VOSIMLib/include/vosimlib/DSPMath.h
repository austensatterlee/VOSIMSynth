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

#pragma once
#include "vosimlib/common.h"
#include <regex>

#define SYN_PI 3.14159265358979323846264338327950288
#define SYN_FIND(CONTAINER, ELEM) std::find(CONTAINER.cbegin(), CONTAINER.cend(), ELEM)
#define SYN_FIND_IF(CONTAINER, ELEM) std::find_if(CONTAINER.cbegin(), CONTAINER.cend(), ELEM)
#define SYN_CONTAINS(CONTAINER, ELEM) SYN_FIND(CONTAINER, ELEM)!=CONTAINER.cend()
#define SYN_CONTAINS_IF(CONTAINER, ELEM) SYN_FIND_IF(CONTAINER, ELEM)!=CONTAINER.cend()
#define SYN_VEC_FIND(VEC, ELEM) SYN_FIND(VEC, ELEM) - VEC.begin()
#define SYN_VEC_FIND_IF(VEC, ELEM) SYN_FIND_IF(VEC, ELEM) - VEC.begin()

namespace syn
{
    /**
     * Linearly interpolate between pt1 and pt2.
     */
    template <typename T>
    T LERP(T pt1, T pt2, T frac) {
        return pt2 * frac + pt1 * (1 - frac);
    }

    /**
     * Scale lerped_pt from a value between pt1 and pt2 to a value between 0 and 1.
     */
    template <typename T>
    T INVLERP(T pt1, T pt2, T lerped_pt) {
        return (lerped_pt - pt1) / (pt2 - pt1);
    }

    /**
     * Clamps val to be in the range [min_val, max_val].
     */
    template <typename T>
    constexpr T CLAMP(T val, T min_val, T max_val) {
        return val < min_val ? min_val : (val > max_val ? max_val : val);
    }

    template <typename T>
    constexpr T MAX(T val1, T val2) {
        return val1 < val2 ? val2 : val1;
    }

    template <typename T>
    constexpr T MIN(T val1, T val2) {
        return val1 > val2 ? val2 : val1;
    }

    /**
     * Computes x modulo m 
     */
    template <typename T>
    T WRAP(T x, T m=1.0) {
        while (x >= m)
            x -= m;
        while (x < 0)
            x += m;
        return x;
    }

    /**
     * Wraps a number to be in the given range
     * \param x the number to wrap
     * \param left_m the left (minimum) boundary
     * \param right_m the right (maximum) boundary
     */
    template <typename T>
    T WRAP2(T x, T left_m, T right_m) {
        const T m = right_m - left_m;
        while (x >= right_m)
            x -= m;
        while (x < left_m)
            x += m;
        return x;
    }

    template <typename T>
    T sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }

    template <typename T>
    bool isDenormal(T a_x) {
        constexpr auto minVal = std::numeric_limits<T>::min;
        constexpr T epsilon = minVal();
        return (a_x != 0 && std::abs(a_x) <= epsilon);
    }

    /////////////////////////////////////
    // Time unit conversion functions  //
    /////////////////////////////////////

    /**
     * Convert pitch (midi note, 0-127) to frequency 
     */
    double pitchToFreq(double pitch);
    double naive_pitchToFreq(double pitch);

    double bpmToFreq(double bpm, double tempo);

    double freqToSamples(double freq, double fs);

    double periodToSamples(double seconds, double fs);

    double samplesToPitch(double samples, double fs);

    double samplesToFreq(double samples, double fs);

    double samplesToPeriod(double samples, double fs);
    
    double samplesToBPM(double samples, double fs, double tempo);

    double lin2db(double lin);
    double db2lin(double db);

    /////////////////////////
    // Waveform generators //
    /////////////////////////

    /**
    * Generates a sample from a blackman-harris window.
    * \param a_k the index of the sample to generate, in the range [0,a_winSize).
    * \param a_winSize size of the window in samples
    */
    double blackman_harris(int a_k, size_t a_winSize);
    double hanning(int a_k, size_t a_winSize);

    double naive_tri(double a_phase);
    double naive_saw(double a_phase);
    double naive_square(double a_phase);    


    template<typename T>
    T fast_tanh_rat(T x) {
        const double ax = abs(x);
        const double x2 = x * x;
        const double z = x * (0.773062670268356 + ax + (0.757118539838817 + 0.0139332362248817 * x2 * x2) * x2 * ax);
        return(z / (0.795956503022967 + abs(z)));        
    }

    //////////////////
    // Misc. utils  //
    //////////////////

    /**
     * \brief Add or increment a number at the end of a string.
     * If the string ends with a number, this function creates a new string with that number incremented by one.
     * If the string does not end with a number, this function returns a new string with "_0" concatenated at the end.
     */
    std::string incrementSuffix(const std::string& a_str, const std::string& a_sep=" ");    
}