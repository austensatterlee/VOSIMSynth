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
#include <vosimlib/Unit.h>
#include <vosimlib/VoiceManager.h>
#include <vosimlib/CircularView.h>
#include <nanogui/widget.h>

#define MAX_SCOPE_BUFFER_SIZE 96000

namespace synui {
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
        syn::CircularView<double> OscilloscopeUnit::getBuffer(int a_bufIndex) const;
        int getDecimationFactor() const { return m_decimationFactor; }

        void reset() override;

    protected:
        void onParamChange_(int a_paramId) override;
        void onInputConnection_(int a_inputPort) override;
        void onInputDisconnection_(int a_inputPort) override;
        void process_() override;

    private:
        void _processPeriodic();
        void _processNonPeriodic();
        void _sync();

    private:
        int m_readIndex;
        int m_writeIndex;
        int m_bufferSize;
        int m_numBuffers;
        int m_decimationFactor;
        int m_subsampleCounter;
        double m_lastPhase;
        int m_samplesSinceLastSync;
        int m_syncCount;
        vector<Eigen::VectorXd> m_buffers;
        int m_iPhase;
    };

    /**
     * Displays the contents of an OscilloscopeUnit on a graph.
     */
    class OscilloscopeWidget : public nanogui::Widget {
        friend class OscilloscopeUnit;
        using Color = nanogui::Color;
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        OscilloscopeWidget(Widget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
            : Widget(a_parent),
              m_vm(a_vm),
              m_unitId(a_unitId),
              m_yMin(-1.0),
              m_yMax(1.0),
              m_autoAdjustSpeed(60.0),
              m_values(),
              m_sideMargin(30),
              m_bottomMargin(20),
              m_topMargin(20) {}

        void draw(NVGcontext* ctx) override;
        void drawGrid(NVGcontext* ctx);

        Eigen::Vector2i OscilloscopeWidget::preferredSize(NVGcontext *) const override { return Eigen::Vector2i(400, 200); }

        const string &caption() const { return m_caption; }
        void setCaption(const string &caption) { m_caption = caption; }

        const string &header() const { return m_header; }
        void setHeader(const string &header) { m_header = header; }

        const string &footer() const { return m_footer; }
        void setFooter(const string &footer) { m_footer = footer; }

        const Color &backgroundColor() const { return m_bgColor; }
        void setBackgroundColor(const Color &backgroundColor) { m_bgColor = backgroundColor; }

        const Color &foregroundColor() const { return m_fgColor; }
        void setForegroundColor(const Color &foregroundColor) { m_fgColor = foregroundColor; }

        const Color &textColor() const { return m_textColor; }
        void setTextColor(const Color &textColor) { m_textColor = textColor; }

        syn::CircularView<double> values() const { return m_values; }
        void setValues(const syn::CircularView<double>& values) { m_values = values; }

        int getUnitId() const { return m_unitId; }
        void setUnitId(int a_id) { m_unitId = a_id; }

    protected:
        /**
         * Smoothly update the y-axis viewing limits according to the auto adjust speed. Should be called
         * every time the buffer is updated.
         */
        void updateYBounds_(float a_yMin, float a_yMax);

        /**
         * Transform a signal point to a screen point.
         */
        float toScreen_(float a_yPt);
        
        /**
         * Transform a screen point to a signal point.
         */
        float fromScreen_(float a_yScreen);

    protected:
        syn::VoiceManager* m_vm;
        int m_unitId;
        double m_yMin, m_yMax;
        double m_autoAdjustSpeed;
        syn::CircularView<double> m_values;

        int m_sideMargin, m_bottomMargin, m_topMargin;
        string m_caption, m_header, m_footer;
        nanogui::Color m_bgColor, m_fgColor, m_textColor;
    };
}