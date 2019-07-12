#ifndef _worker_h_included
#define _worker_h_included


#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <string.h>
#include <atomic>

class Pool;
/*
 * This represents a worker thread.
 * The main action happens in Loop().
 *
 * Loop() waits on Cv.
 * A dispatch calls Add() to add lines to process and
 * notifies the CV when it has completed adding lines with Release()
 *
 * To wind up execution, the Drain() method is called.
 */
class Worker
{
    public:
    Worker(int id, Pool*p);
    static void RunWrap(Worker* c);
    void Add(const char* l);
    void Loop();
    void Release();
    void Drain();
    int GetId() { return this->myId; }

    private:
    std::vector<std::string> lines;
    std::mutex linelock;
    std::condition_variable rungate;
    std::mutex runlock;
    std::mutex stoplock;
    int myId;
    Pool* workerPool;
    std::atomic<bool> keepGoing;
};

#endif