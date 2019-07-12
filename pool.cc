#include <mutex>
#include <map>
#include "pool.h"
#include "worker.h"

Pool::Pool() {}

/*
 * Add a worker to the pool, initially set it to idle
 */
void Pool::AddWorker(Worker* w, int widx)
{
    std::lock_guard<std::mutex> locked(this->wlock);
    this->idleWorkers[widx] = w;
}


/*
 * Move a worker from the busy list to the idle list.
 */
void Pool::SetIdle(int widx)
{
    Worker* t;

    std::lock_guard<std::mutex> locked(this->wlock);

    // need the worker. it SHOULD be in the busy list.
    if ( this->busyWorkers.find(widx) != this->busyWorkers.end() )
    {
	t = this->busyWorkers[widx];
	this->busyWorkers.erase(widx);
	this->idleWorkers[widx] = t;
	return;
    }

    // Okay. maybe just calling twice?
    if ( this->idleWorkers.find(widx) != this->idleWorkers.end() )
    {
	return;
    }

    else
    {
	// ???
    }
}


/*
 * Move a worker from the idle list to the busy list
 */
void Pool::SetBusy(int widx)
{
    Worker* t;

    std::lock_guard<std::mutex> locked(this->wlock);

    // need the worker. it SHOULD be in the idle list.
    if ( this->idleWorkers.find(widx) != this->idleWorkers.end() )
    {
	t = this->idleWorkers[widx];
	this->idleWorkers.erase(widx);
	this->busyWorkers[widx] = t;
	return;
    }

    // Okay. maybe just calling twice?
    if ( this->busyWorkers.find(widx) != this->busyWorkers.end() )
    {
	return;
    }

    else
    {
	// ???
    }
}


/*
 * Get a worker from the idle worker list
 * The worker will need to set itself as busy.
 */
Worker* Pool::GetIdleWorker()
{
    Worker* w;
    std::lock_guard<std::mutex> locked(this->wlock);
    

    auto it = this->idleWorkers.begin();
    w = it->second;
    this->idleWorkers.erase(it->first);
    return w;
}


