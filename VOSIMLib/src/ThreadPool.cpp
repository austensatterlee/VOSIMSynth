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
#ifdef _DEBUG
            std::ostringstream oss1;
            oss1 << boost::this_thread::get_id() << " \"S\" " << task << std::endl;
            OutputDebugString(oss1.str().c_str());
            return;
#endif
        }

        /* Execute the received task */
#ifdef _DEBUG
        std::ostringstream oss1;
        oss1 << boost::this_thread::get_id() << " \"R\" " << task << std::endl;
        OutputDebugString(oss1.str().c_str());
#endif

        (*task)();
        
        /* Notify thread pool that we are done. */
#ifdef _DEBUG
        std::ostringstream oss2;
        oss2 << boost::this_thread::get_id() << " \"C\" " << task << std::endl;
        OutputDebugString(oss2.str().c_str());    
#endif   

        wdata->pool.notifyJobComplete();
    }    
}
