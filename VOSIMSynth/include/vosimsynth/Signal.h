/*
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
along with VOSIMProject.  If not, see <http://www.gnu.org/licenses/>.

Copyright 2016, Austen Satterlee
*/

#pragma once

#include <functional>
#include <map>

namespace synui {
    /** 
     * A signal object may call multiple slots with the
     * same signature. You can connect functions to the signal
     * which will be called when the emit() method on the
     * signal object is invoked. Any argument passed to emit()
     * will be passed to the given functions.
     */
    template <typename... Args>
    class Signal {

    public:
        Signal() : current_id_(0) {}

        // Non-copyable
        Signal(const Signal&) = delete;
        Signal& operator=(const Signal&) = delete;

        // connects a member function to this Signal
        template <typename X, typename Y>
        int connect_member(X* inst, void (Y::*func)(Args ...)) {
            return connect([=](Args ... args) {
            (inst ->* func)(args...);
        });
        }

        // connects a const member function to this Signal
        template <typename X, typename Y>
        int connect_member(X* inst, void (Y::*func)(Args ...) const) {
            return connect([=](Args ... args) {
            (inst ->* func)(args...);
        });
        }

        // connects a std::function to the signal. The returned
        // value can be used to disconnect the function again
        int connect(const std::function<void(Args ...)>& slot) const {
            slots_.insert(std::make_pair(++current_id_, slot));
            return current_id_;
        }

        // disconnects a previously connected function
        void disconnect(int id) const {
            slots_.erase(id);
        }

        // disconnects all previously connected functions
        void disconnect_all() const {
            slots_.clear();
        }

        // calls all connected functions
        void emit(Args... p) {
            for (auto it : slots_) {
                it.second(p...);
            }
        }

        void operator()(Args... p){
            emit(p...);
        }

    private:
        mutable std::map<int, std::function<void(Args ...)>> slots_;
        mutable int current_id_;
    };
}
