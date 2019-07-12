#include <mutex>
#include <map>
#include "pool.h"
#include "worker.h"
#include <chrono>

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
 * Delete worker from all pools.
 */
void Pool::DelWorker(int widx)
{
    std::lock_guard<std::mutex> locked(this->wlock);
    if ( this->busyWorkers.find(widx) != this->busyWorkers.end() )
    {
	this->busyWorkers.erase(widx);
    }
    if ( this->idleWorkers.find(widx) != this->idleWorkers.end() )
    {
	this->idleWorkers.erase(widx);
    }
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
 * This will also mark the worker as busy.
 */
Worker* Pool::GetIdleWorker()
{
    Worker* w;
    int id;
    std::lock_guard<std::mutex> locked(this->wlock);
    

    auto it = this->idleWorkers.begin();
    if ( it == this->idleWorkers.end() )
    {
	//fprintf(stderr, "No idle workers.\n");
	return NULL;
    }
    w = it->second;
    id = it->first;
    this->idleWorkers.erase(id);
    this->busyWorkers[id] = w;
    return w;
}


/*
 * Blocks until there are idle workers available or
 * some short period of time.
 *
 * Make sure to re-check the result of GetIdleWorker() as it
 * may still return NULL.
 */
void Pool::WaitForIdleWorker()
{
    std::unique_lock<std::mutex> wglw(this->wglock);
    this->waitgate.wait_for(wglw, std::chrono::milliseconds(10));
}


/*
 * Shut down all the workers
 * Blocks until all workers have shut down.
 * This will also join() their threads.
 */
void Pool::Shutdown()
{
    Worker* w;
    std::thread* t;
    int qsize;

    // keep whacking till the moles are gone
    while(1)
    {
	// kill threads from the idle queue
	// the busy threads are busy; leave them alone!
	w = this->GetIdleWorker();
	if ( w != NULL )
	{
	    t = w->GetThread();
	    fprintf(stderr, "Draining and joining worker %d (%x)\n", w->GetId(), w);
	    w->Drain();
	    t->join();
	} else // no more idle workers
	{
	    // check busy queue, if it's empty, we're done.
	    this->wlock.lock();
	    qsize = this->busyWorkers.size();
	    this->wlock.unlock();
	    //fprintf(stderr, "Waiting for %d more busy workers\n", qsize);
	    if ( qsize == 0 )
	    {
		return;
	    }
	}
	//sleep(1);
    }
}
