#ifndef _pool_h_included_
#define _pool_h_included_

#include "worker.h"
#include <map>
#include <mutex>

/*
 * Thread pool
 * Threads can check themeselves in/out as necessary
 */
class Pool
{
    public:
    Pool();
    void AddWorker(Worker* w, int widx);
    void SetIdle(int widx);
    void SetBusy(int widx);
    void WaitForIdleWorker();
    Worker* GetIdleWorker();

    private:
    std::map<int, Worker*> idleWorkers;
    std::map<int, Worker*> busyWorkers;
    std::mutex wlock;
    std::condition_variable waitgate;
    std::mutex wglock;


};


#endif
