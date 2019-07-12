#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <string.h>
#include "pool.h"
#include "worker.h"

Worker::Worker(int id, Pool* p)
{
    this->keepGoing = true;
    this->myId = id;
    this->workerPool = p;

    this->workerPool->AddWorker(this, this->myId);
}

/*
 * Use this as the thread entry point
 */
void Worker::RunWrap(Worker* c)
{
    c->Loop();
}

/*
 * Add a line to the input buffer
 */
void Worker::Add(const char* l)
{
    this->linelock.lock();
    this->lines.push_back(l);
    this->linelock.unlock();
}

/*
 * The main loop of the thread. Don't call this directly.
 */
void Worker::Loop()
{
    bool run = true;

    // wrappers for mutex since we need to release locks before the scope
    // disappears
    std::unique_lock<std::mutex> rlw(this->runlock);
    //std::unique_lock<std::mutex> slw(this->stoplock);
    while(run)
    {
	this->rungate.wait(rlw);
	this->workerPool->SetBusy(this->myId);
	this->stoplock.lock();
	run = this->keepGoing;
	this->stoplock.unlock();
	this->linelock.lock();
	for ( std::string & s : this->lines )
	{
	    //fprintf(stderr,"Got %s", s.c_str());
	}
	this->lines.clear();
	this->linelock.unlock();
	this->workerPool->SetIdle(this->myId);
    }
}


/*
 * Call this to process whatever is in the input queue and exit
 */
void Worker::Drain()
{
    this->stoplock.lock();
    this->keepGoing = false;
    this->stoplock.unlock();
    this->Release();
}

/*
 * Call this to process the batch of added lines
 */
void Worker::Release()
{
    fprintf(stderr,"Thread released\n");
    this->rungate.notify_one();
}
