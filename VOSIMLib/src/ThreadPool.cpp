#include "ThreadPool.h"

void syn::runThreadWorker(WorkerData* wdata)
{
    ThreadJob* task;
    while (true)
    {        
        boost::unique_lock<boost::mutex> lck(wdata->mtx); 

        /* Wait until either a real job is dispatched or the thread pool is stopped. */
        wdata->pool.pop(task);
        if(task==nullptr){ 
            // ThreadPool is stopped; commit suicide.
#if defined(_DEBUG) && defined(_WINDOWS)
            std::ostringstream oss1;
            oss1 << boost::this_thread::get_id() << " \"S\" " << task << std::endl;
            OutputDebugString(oss1.str().c_str());
#endif
            return;
        }

        /* Execute the received task */
#if defined(_DEBUG) && defined(_WINDOWS)
        std::ostringstream oss1;
        oss1 << boost::this_thread::get_id() << " \"R\" " << task << std::endl;
        OutputDebugString(oss1.str().c_str());
#endif

        (*task)();
        
        /* Notify thread pool that we are done. */
#if defined(_DEBUG) && defined(_WINDOWS)
        std::ostringstream oss2;
        oss2 << boost::this_thread::get_id() << " \"C\" " << task << std::endl;
        OutputDebugString(oss2.str().c_str());    
#endif   

        wdata->pool.notifyJobComplete();
    }    
}

void syn::ThreadPool::notifyJobComplete()
{
    boost::unique_lock<boost::mutex> dlck(m_dispatchMtx);
    m_dispatchedWorkers--;
    m_finishCV.notify_all();
}

void syn::ThreadPool::pop(ThreadJob*& item)
{
    boost::unique_lock<boost::mutex> lck(m_dispatchMtx);
    while (!m_stopped && m_tasks.empty())
        m_dispatchCV.wait(lck);
    if (m_stopped)
    {
        item = nullptr;
    }
    else
    {
        m_tasks.pop(item);
        m_dispatchedWorkers++;
    }
}

void syn::ThreadPool::push(ThreadJob* item)
{
    boost::unique_lock<boost::mutex> lck(m_dispatchMtx);
    m_tasks.push(item);
    m_dispatchCV.notify_one();
}

void syn::ThreadPool::waitForWorkers()
{
    
    ThreadJob* item;
    while(m_tasks.pop(item)){
        (*item)();
    }        

    boost::unique_lock<boost::mutex> lck(m_dispatchMtx);
    while (m_dispatchedWorkers > 0){
        m_finishCV.wait(lck);
    }

#if defined(_DEBUG) && defined(_WINDOWS)
    std::ostringstream oss;
    oss << boost::this_thread::get_id() << " synced" << std::endl;
    OutputDebugString(oss.str().c_str());
#endif
}

void syn::ThreadPool::resizePool(int a_newSize)
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

void syn::ThreadPool::_stop()
{
    boost::unique_lock<boost::mutex> lck(m_dispatchMtx);
    m_stopped = true;
    m_dispatchCV.notify_all();
}

void syn::ThreadPool::_start()
{
    boost::unique_lock<boost::mutex> lck(m_dispatchMtx);
    m_stopped = false;
}
