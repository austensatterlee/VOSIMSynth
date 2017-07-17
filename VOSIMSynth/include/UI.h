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
*  \file UI.h
*  \brief Common UI functions and definitions
*  \details Partly derived from NanoGUI source: https://github.com/wjakob/nanogui
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UI__
#define __UI__
#include "DSPMath.h"
#include "VOSIMSynth/common.h"
#include <nanogui/common.h>
#include <eigen/Core>
#include <functional>

namespace synui
{
    using Eigen::Vector2f;
    using Eigen::Vector3f;
    using Eigen::Vector4f;
    using Eigen::Vector2i;
    using Eigen::Vector3i;
    using Eigen::Vector4i;
    using Eigen::Matrix3f;
    using Eigen::Matrix4f;
    using Eigen::VectorXf;
    using Eigen::MatrixXf;

    typedef Eigen::Matrix<uint32_t, 2, 1> Vector2u;
    typedef Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> MatrixXu;

    /**
     * Finds the point closest to the given 'pt' which lies on the line defined by points 'a' and 'b'.
     */
    template <typename T>
    Eigen::Matrix<T, 2, 1> closestPointOnLine(const Eigen::Matrix<T, 2, 1>& pt, const Eigen::Matrix<T, 2, 1>& a, const Eigen::Matrix<T, 2, 1>& b) {
        double ablength = (a - b).norm();
        Eigen::Vector2d abnorm = (a - b).template cast<double>() * (1.0 / ablength);
        double proj = (pt - b).template cast<double>().dot(abnorm);
        proj = syn::CLAMP<double>(proj, 0, ablength);
        return (b.template cast<double>() + abnorm * proj).template cast<T>();
    }

    template <typename T>
    double pointLineDistance(const Eigen::Matrix<T, 2, 1>& pt, const Eigen::Matrix<T, 2, 1>& a, const Eigen::Matrix<T, 2, 1>& b) {
        return (pt - closestPointOnLine(pt, a, b)).norm();
    }


    /**
     * \brief Given a string representation of a number, compute the location of the first non-zero digit relative to the decimal point.
     * Examples:
     *  firstDigit("1000.00")   ->  4
     *  firstDigit("1000")      ->  4
     *  firstDigit("0.0001")    -> -4
     *  firstDigit("1.0")       ->  1
     *  firstDigit("0.1")       -> -1
     *  firstDigit("0.0")       ->  0
     * \return Location of the first non-zero digit relative to the decimal point. Negative values indicate locations to the right of the decimal point.
     */
    inline int firstNonzero(const std::string& a_num)
    {
        size_t decimal_pos = a_num.find_first_of(".");
        if (decimal_pos == std::string::npos)
            decimal_pos = a_num.size()-1;
        size_t nonzero_pos = a_num.find_first_not_of("0");
        if(nonzero_pos == std::string::npos)
            return 0;
        decimal_pos = a_num.size()-1 - decimal_pos;
        nonzero_pos = a_num.size()-1 - nonzero_pos;
        return static_cast<int>(nonzero_pos-decimal_pos);
    }

    /// Determine whether an icon ID is a texture loaded via nvgImageIcon
    inline bool nvgIsImageIcon(int value) {
        return value < 1024;
    }

    /// Determine whether an icon ID is a font-based icon (e.g. from the entypo.ttf font)
    inline bool nvgIsFontIcon(int value) {
        return value >= 1024;
    }
    
    /**
     * \brief Draw a shadow around a rectangular area.
     * \param r corner radius
     * \param s shadow size
     * \param f feather percentage
     * \param a_shadowColor shadow (inner) color
     * \param a_transparentColor transparent (outer) color
     */
    void drawRectShadow(NVGcontext* ctx, float x, float y, float w, float h, float r = 1.0f, float s = 5.0f, float f=1.0f, const nanogui::Color& a_shadowColor = {0.0f, 0.5f}, const nanogui::Color& a_transparentColor = {0.0f, 1.0f});
    void drawRadialShadow(NVGcontext* ctx, float x, float y, float r, float s, float f, const nanogui::Color& a_shadowColor = {0.0f, 0.5f}, const nanogui::Color& a_transparentColor = {0.0f, 1.0f});

    void drawTooltip(NVGcontext* a_ctx, const Vector2i& a_pos, const std::string& a_str, double elapsed);
}
#endif
