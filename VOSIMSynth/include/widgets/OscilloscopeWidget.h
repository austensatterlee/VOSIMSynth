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
        const Eigen::VectorXf& OscilloscopeUnit::getBuffer(int a_bufIndex) const;
        int getBufferSize(int a_bufIndex) const;

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

    class OscilloscopeWidget : public nanogui::Graph {
    public:
        OscilloscopeWidget(Widget* a_parent, syn::VoiceManager* a_vm)
            : Graph(a_parent, "Oscilloscope"),
              m_vm(a_vm),
              m_unitId(-1) 
        {
            setFixedSize({400,400});    
        }

        void draw(NVGcontext* ctx) override;
    private:
        friend class OscilloscopeUnit;
        syn::VoiceManager* m_vm;
        int m_unitId;
    };
}
