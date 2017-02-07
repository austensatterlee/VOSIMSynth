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
 *  \file Logger.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 01/2017
 */

#pragma once
#include <vector>
#include <functional>

namespace synui
{
    class Logger
    {
    public:
        typedef std::function<void()> Listener;

        static Logger& instance();

        void log(const std::string& a_channel, const std::string& a_msg);

        const std::vector<std::pair<std::string, std::string>>& getLogs() const;

        void addListener(Listener a_listener);

        void reset();

    private:
        Logger();
        std::vector<std::pair<std::string, std::string> > m_logs;
        std::vector<Listener> m_listeners;
        int m_maxLogs;
        int m_nextIndex;
    };
}