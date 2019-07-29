#ifndef _worker_h_included
#define _worker_h_included


#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <string.h>
#include <atomic>
#include "brokersaurusrex.h"
#include <broker/broker.hh>
#include <broker/bro.hh>

#define LOGINFO_FORWARD_RE "^(?P<start_time>\\d+)\\s+(?P<relayhost>\\S+)\\s+(?:Message )?forwarded from (?P<hostname>\\S+):"

#define LOGINFO_NOFORWARD_RE "^(?P<start_time>\\d+)\\s+(?P<hostname>\\S+):?"



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
    void Compile(RexManifest*);
    void Loop();
    void Release();
    void Drain();
    int GetId() { return this->myId; }

    void SetThread(std::thread *th) { this->myThread = th; }
    std::thread* GetThread() { return this->myThread; }

    void SetEndpoint(broker::endpoint* ep) { this->ep = ep; }

    private:
    std::vector<std::string> lines;
    std::mutex linelock;
    std::condition_variable rungate;
    std::mutex runlock;
    int myId;
    std::thread * myThread;
    Pool* workerPool;
    std::atomic<bool> keepGoing;
    RexList rl;
    RexItem* loginfo;
    RexItem* loginfo_forwarded;
    broker::endpoint* ep;
};

#endif
