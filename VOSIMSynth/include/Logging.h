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

#pragma once
#include <IPlug/Log.h>
#include <chrono>

#if defined(TRACER_BUILD)
#define TIME_TRACE synui::TimingTracer __trcr(TRACELOC, "");
#else
#define TIME_TRACE
#endif

namespace synui
{
    class TimingTracer {
    public:
        template <typename ...Msg>
        TimingTracer(const char* a_funcName, int a_line, const char* a_fmtStr, Msg... a_msg);
        virtual ~TimingTracer();
    private:
        void _recordStartTime();
    private:
        std::chrono::steady_clock::time_point m_startTime;
        std::string m_funcName;
        int m_line;
    };

    template <typename ...Msg>
    TimingTracer::TimingTracer(const char* a_funcName, int a_line, const char* a_fmtStr, Msg... a_msg)
        : m_funcName(a_funcName),
          m_line(a_line) 
    {
        Trace(a_funcName, a_line, a_fmtStr, a_msg...);
        _recordStartTime();
    }
}
