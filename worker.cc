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
    this->keepGoing.store(true, std::memory_order_relaxed);
    this->myId = id;
    this->workerPool = p;

    this->workerPool->AddWorker(this, this->myId);
    //fprintf(stderr,"Init worker %d at %x\n", id, this);
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
    //fprintf(stderr,"Add() worker %d at %x\n", this->myId, this);
    this->linelock.lock();
    this->lines.push_back(l);
    this->linelock.unlock();
}

/*
 * The main loop of the thread. Don't call this directly.
 */
void Worker::Loop()
{

    // wrappers for mutex since we need to release locks before the scope
    // disappears
    std::unique_lock<std::mutex> rlw(this->runlock);
    while(this->keepGoing.load(std::memory_order_relaxed))
    {
	this->rungate.wait(rlw);
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
    this->keepGoing.store(false, std::memory_order_relaxed);
    this->Release();
}

/*
 * Call this to process the batch of added lines
 */
void Worker::Release()
{
    this->rungate.notify_one();
}
