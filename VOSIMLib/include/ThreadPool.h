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
#include <windows.h>

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

        virtual ~ThreadPool() { };

        void notifyJobComplete()
        {
            boost::unique_lock<boost::mutex> dlck(m_dispatchMtx);
            m_dispatchedWorkers--;
            m_finishCV.notify_all();
        }

        void pop(ThreadJob*& item)
        {
            boost::unique_lock<boost::mutex> lck(m_dispatchMtx);
            while(!m_stopped && m_tasks.empty())
                m_dispatchCV.wait(lck);
            if(m_stopped){
                item = nullptr;
            }else{
                m_tasks.pop(item);
                m_dispatchedWorkers++;
            }
        }

        void push(ThreadJob* item)
        {
            boost::unique_lock<boost::mutex> lck(m_dispatchMtx);
            m_tasks.push(item);
            m_dispatchCV.notify_one();
        }

        void waitForWorkers()
        {
            boost::unique_lock<boost::mutex> lck(m_dispatchMtx);

            while(!m_tasks.empty() || m_dispatchedWorkers>0)
                m_finishCV.wait(lck);
            
#ifdef _DEBUG
            std::ostringstream oss;
            oss << boost::this_thread::get_id() << " synced" << std::endl;
            OutputDebugString(oss.str().c_str());
#endif
        }

        void resizePool(int a_newSize)
        {
            // Wait for any active workers to finish their tasks.
            waitForWorkers();
            // Send stop signal.
            _stop();
            for (auto& worker : m_workers)
            {
                worker->thread.join();
                delete worker;
            }

            // Reset the stop flag.
            _start();
            // Create new workers.
            m_workers.resize(a_newSize);
            for (int i = 0; i < a_newSize; i++)
            {
                m_workers[i] = new WorkerData(*this);
                m_workers[i]->thread = boost::thread(runThreadWorker, m_workers[i]);
            }
        }

    private:
        void _stop()
        {
            boost::unique_lock<boost::mutex> lck(m_dispatchMtx);
            m_stopped = true;
            m_dispatchCV.notify_all();
        }
        void _start()
        {
            boost::unique_lock<boost::mutex> lck(m_dispatchMtx);
            m_stopped = false;
        }
    };
}
