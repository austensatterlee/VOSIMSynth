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
     * Abstract zero-argument functor with no return value.
     * \todo Could this just be replaced with std::function<void()>?
     */
    class VOSIMLIB_API Command {
    public:
        virtual ~Command() = default;
        virtual void operator()() = 0;
    };
    
    template <class T>
    class VOSIMLIB_API CommandImplem : public Command {
        std::function<void()> m_f;
    public:
        CommandImplem(T&& a_f)
            : m_f(std::forward<T>(a_f)) {}

        void operator()() override { m_f(); }
    };

    template <class T1, class T2>
    class VOSIMLIB_API ChainedCommand : public Command {
        std::function<void()> m_f1;
        std::function<void()> m_f2;
    public:
        ChainedCommand(T1&& a_f1, T2&& a_f2)
            : m_f1(std::forward<T1>(a_f1)),
              m_f2(std::forward<T2>(a_f2)) {}
        void operator()() override { m_f1(); m_f2(); }
    };

    template <class T>
    Command* MakeCommand(T&& a_f) { return new CommandImplem<T>(std::forward<T>(a_f)); }

    template<typename First, typename Second, typename... Rest>
    Command* ChainCommands(First&& a_first, Second&& a_second, Rest&&... a_rest) {
        ChainedCommand<First, Second> chain(a_first, a_second);
        return ChainCommands(std::forward<ChainedCommand<First, Second>>(chain), std::forward<Rest...>(a_rest...));
    }

    template<typename First, typename Second>
    Command* ChainCommands(First&& a_first, Second&& a_second) {
        return new ChainedCommand<First, Second>(std::forward<First>(a_first), std::forward<Second>(a_second));
    }
}
