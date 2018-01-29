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
#include <chrono>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>

#if defined(TRACER_BUILD)
#define TIME_TRACE syn::TimingTracer __trcr(__FUNCTION__, __LINE__, "");
#else
#define TIME_TRACE
#endif

#if !defined(LOGFILE)
#define LOGFILE "log.txt"
#endif

namespace syn
{
    class GlobalLogFile {
        GlobalLogFile() { setFile(LOGFILE); }
    public:
        GlobalLogFile(const GlobalLogFile&) = delete;
        void operator=(const GlobalLogFile&) = delete;

        void setFile(const std::string& a_fn) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_fs.is_open()) m_fs.close();
            m_fs.open(a_fn);
        }

        template <typename ...Msg>
        void log(const std::string& a_funcName, int a_line, Msg&&... a_msg) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_fs << "[" << std::this_thread::get_id() << ":" << a_funcName << ":" << a_line << "] ";
            using expander = int[sizeof...(Msg)];
            expander{ (m_fs << a_msg, 0)... };
            m_fs << std::endl;
            m_fs.flush();
        }

        static GlobalLogFile& instance() {
            static GlobalLogFile singleton;
            return singleton;
        }

        ~GlobalLogFile() {
            std::lock_guard<std::mutex> lock(m_mutex);
            if(m_fs.is_open()) m_fs.close();
        }
    private:
        std::mutex m_mutex;
        std::ofstream m_fs;
    };

    class TimingTracer {
    public:
        template <typename ...Msg>
        TimingTracer(const std::string& a_funcName, int a_line, Msg&&... a_msg);
        ~TimingTracer();
    private:
        void _recordStartTime();
    private:
        std::chrono::steady_clock::time_point m_startTime;
        std::string m_funcName;
        int m_line;
    };

    template <typename ...Msg>
    TimingTracer::TimingTracer(const std::string& a_funcName, int a_line, Msg&&... a_msg)
        : m_funcName(a_funcName),
          m_line(a_line) 
    {
        GlobalLogFile::instance().log(a_funcName, a_line, std::forward<Msg>(a_msg)...);
        _recordStartTime();
    }
}
