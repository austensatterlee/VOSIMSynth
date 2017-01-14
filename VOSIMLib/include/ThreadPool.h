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
 *  \file ThreadPool.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 01/2017
 */

#pragma once
#include "Circuit.h"
#include "CircularContainers.h"
#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>

#if defined(_DEBUG) && defined(_WINDOWS)
#include <windows.h>
#endif

namespace syn
{
    struct VOSIMLIB_API ThreadJob
    {
        virtual ~ThreadJob() = default;
        virtual void operator()() = 0;
    };

    struct VOSIMLIB_API CircuitThreadJob : ThreadJob
    {
        Circuit* circuit = nullptr;

        void operator()() override
        {
            circuit->tick();
        }
    };

    class ThreadPool;

    struct WorkerData
    {
        WorkerData(ThreadPool& a_pool) : pool(a_pool) {}
        ThreadPool& pool;
        boost::thread thread;
        boost::mutex mtx;
    };

    void runThreadWorker(WorkerData* wdata);

    class VOSIMLIB_API ThreadPool
    {
        friend void runThreadWorker(WorkerData* wdata);
        boost::lockfree::queue<ThreadJob*> m_tasks;
        std::vector<WorkerData*> m_workers;

        int m_dispatchedWorkers;
        boost::mutex m_dispatchMtx;
        boost::condition_variable m_dispatchCV;
        boost::condition_variable m_finishCV;

        bool m_stopped;
    public:
        ThreadPool(size_t max_jobs) :
            m_tasks{max_jobs},
            m_dispatchedWorkers(0),
            m_stopped(false) {};

        virtual ~ThreadPool()
        {
            resizePool(0);
        };

        void notifyJobComplete();

        void pop(ThreadJob*& item);

        void push(ThreadJob* item);

        void waitForWorkers();

        void resizePool(int a_newSize);

    private:
        void _stop();

        void _start();
    };
}
