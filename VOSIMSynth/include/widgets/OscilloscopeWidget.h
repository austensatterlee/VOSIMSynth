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
 *  \file OscilloscopeWidget.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 04/2017
 */

#pragma once
#include "Unit.h"
#include "VoiceManager.h"
#include <nanogui/graph.h>

namespace synui {
    class OscilloscopeWidget;

    /**
     * Collects input samples into a buffer to be displayed by an OscilloscopeWidget.
     */
    class OscilloscopeUnit : public syn::Unit {
        DERIVE_UNIT(OscilloscopeUnit)
    public:
        enum Parameter {
            pBufferSize,
            pNumPeriods
        };

        explicit OscilloscopeUnit(const string& a_name);

        OscilloscopeUnit(const OscilloscopeUnit& a_rhs)
            :
            OscilloscopeUnit(a_rhs.name()) {}

        int getNumBuffers() const;
        Eigen::Map<const Eigen::VectorXf> OscilloscopeUnit::getBuffer(int a_bufIndex) const;

        void reset() override {};

    protected:
        void onParamChange_(int a_paramId) override;
        void MSFASTCALL process_() GCCFASTCALL override;

    private:
        void _sync();

    private:
        friend class OscilloscopeWidget;
        OscilloscopeWidget* m_widget;
        int m_bufferIndex;
        int m_bufferSize;
        int m_numBuffers;
        int m_iPhase;

        vector<Eigen::VectorXf> m_buffers;

        double m_lastPhase;
        int m_lastSync;
        int m_syncCount;
    };

    /**
     * Displays the contents of an OscilloscopeUnit on a graph.
     */
    class OscilloscopeWidget : public nanogui::Graph {
        friend class OscilloscopeUnit;
    public:
        OscilloscopeWidget(Widget* a_parent, syn::VoiceManager* a_vm)
            : Graph(a_parent, "Scope"),
              m_vm(a_vm),
              m_unitId(-1),
              m_yMin(-1.0),
              m_yMax(1.0),
              m_autoAdjustSpeed(60.0)
        {
            setFixedSize({400,400});
        }

        void draw(NVGcontext* ctx) override;
        void drawGrid(NVGcontext* ctx);

        /**
         * Smoothly update the y-axis viewing limits according to the auto adjust speed. Should be called
         * every time the buffer is updated.
         */
        void updateYBounds(float a_yMin, float a_yMax);

        /**
         * Transform a signal point to a screen point.
         */
        float toScreen(float a_yPt);
        
        /**
         * Transform a screen point to a signal point.
         */
        float fromScreen(float a_yScreen);

    protected:
        syn::VoiceManager* m_vm;
        int m_unitId;
        float m_yMin, m_yMax;
        float m_autoAdjustSpeed;
    };
}
