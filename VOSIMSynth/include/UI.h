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
#include <eigen/Core>
#include <nanogui/nanogui.h>
#include <IPlug/Containers.h>
#include <functional>
#include <vector>
#include <memory>
#include <entypo.h>

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
        int decimal_pos = a_num.find_first_of(".");
        if (decimal_pos == std::string::npos)
            decimal_pos = a_num.size()-1;
        int nonzero_pos = a_num.find_first_not_of("0");
        if(nonzero_pos == std::string::npos)
            return 0;
        decimal_pos = a_num.size()-1 - decimal_pos;
        nonzero_pos = a_num.size()-1 - nonzero_pos;
        return nonzero_pos-decimal_pos;
    }

    /// Determine whether an icon ID is a texture loaded via nvgImageIcon
    inline bool nvgIsImageIcon(int value) {
        return value < 1024;
    }

    /// Determine whether an icon ID is a font-based icon (e.g. from the entypo.ttf font)
    inline bool nvgIsFontIcon(int value) {
        return value >= 1024;
    }

    template <typename StoreType>
    class CachedComputation
    {
    public:
        CachedComputation<StoreType>() {}

        /**
         * Check that the watched values have not changed since the last time this method was called.
         * If they have, use the provided callable to refresh the cache.
         */
        template <typename... Args>
        bool operator()(const std::function<StoreType(Args...)>& a_func, Args... args) {
            bool refresh_cond = isDirty();
            if (refresh_cond) {
                a_func(args...);
                m_isFirstUpdate = false;
            }
            return refresh_cond;
        }

        template <typename... Args>
        void resetConds(const Args*... args) {
            m_conds.clear();
            addConds(args...);
        }

        template <typename First, typename... Rest>
        void addConds(const First* first, const Rest*... rest) {
            m_conds.push_back(make_unique<UpdateConditionImplem<First>>(first));
            addConds(rest...);
        }

        template <typename Only>
        void addConds(const Only* only) {
            m_conds.push_back(make_unique<UpdateConditionImplem<Only>>(only));
        }

        bool isDirty() const {
            bool needsRefresh = m_isFirstUpdate;
            for (int i = 0; i < m_conds.size(); i++) {
                needsRefresh |= m_conds[i]->update();
            }
            return needsRefresh;
        }

        void setDirty() {
            m_isFirstUpdate = true;
        }

    private:
        class UpdateCondition
        {
        public:
            virtual ~UpdateCondition() {}

            virtual bool update() = 0;
        };

        template <typename T>
        class UpdateConditionImplem : public UpdateCondition
        {
            T lastval;
            const T* currval;
        public:
            explicit UpdateConditionImplem(const T* a_addr)
                : currval(a_addr) {}

            bool update() override {
                bool result = *currval != lastval;
                lastval = *currval;
                return result;
            }
        };

        StoreType cached;
        std::vector<std::unique_ptr<UpdateCondition>> m_conds;
        bool m_isFirstUpdate = true;
    };

    /**
     * Packs an ordered sequence of variables into a byte array. The variables may be of any type.
     * \returns The total number of bytes used to store the arguments.
     */
    template<typename First, typename... Rest>
    int PutArgs(ByteChunk* a_chunk, const First& a_first, const Rest&... a_rest) {
        a_chunk->Put<First>(&a_first);
        return PutArgs(a_chunk, a_rest...);
    }

    template<typename Only>
    int PutArgs(ByteChunk* a_chunk, const Only& a_only) {
        return a_chunk->Put<Only>(&a_only);
    }

    /**
     * Unpacks a byte array into the given sequence of variables, in the order they are given. The variables may be of any type.
     * \returns The next offset of the byte array to be read.
     */
    template<typename First, typename... Rest>
    int GetArgs(ByteChunk* a_chunk, int a_startPos, First& a_first, Rest&... a_rest) {
        a_startPos = a_chunk->Get<First>(&a_first, a_startPos);
        return GetArgs(a_chunk, a_startPos, a_rest...);
    }

    template<typename Only>
    int GetArgs(ByteChunk* a_chunk, int a_startPos, Only& a_only) {
        return a_chunk->Get<Only>(&a_only, a_startPos);
    }

    /* Draw functions */
    
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
