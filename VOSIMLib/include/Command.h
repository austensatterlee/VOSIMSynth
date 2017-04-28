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
#include <functional>
#include "vosimlib/common.h"

namespace syn {
    /**
     * \brief Used to pass messages to a voice manager via VoiceManager::queueAction. 
     */
    class VOSIMLIB_API Command {     
    public:
        virtual ~Command() = default;
        virtual void operator()() = 0;
        virtual void destroy() { delete this; }
    };

    template<class T>
    class VOSIMLIB_API CommandImplem : public Command {   
        std::function<void()> m_f;
    public:
        CommandImplem(T&& f)
            : m_f(std::forward<T>(f)) {}
        void operator()() override { m_f(); }
    };
    
    template <class T>
    Command* MakeCommand(T&& f) { return new CommandImplem<T>(std::forward<T>(f)); }
}
