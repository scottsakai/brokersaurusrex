#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <string.h>
#include "pool.h"
#include "worker.h"
#include <broker/broker.hh>
#include <broker/bro.hh>

Worker::Worker(int id, Pool* p)
{
    this->keepGoing.store(true, std::memory_order_relaxed);
    this->myId = id;
    this->workerPool = p;

    this->workerPool->AddWorker(this, this->myId);
    //fprintf(stderr,"Init worker %d at %x\n", id, this);
}

void Worker::Compile(RexManifest* rm)
{
    RexItem* ri;
    for ( auto p : *rm )
    {
	//fprintf(stderr, "Compiling %s : %s\n", p.first.c_str(), p.second.c_str());
	ri = new RexItem(p.first.c_str(), p.second.c_str());
	this->rl.push_back(ri);
    }

    this->loginfo = new RexItem("loginfo", LOGINFO_NOFORWARD_RE);
    this->loginfo_forwarded = new RexItem("loginfo_forwarded", LOGINFO_FORWARD_RE);
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
 * We're not locking here since Add() should only be called
 * on an idle worker. the only consumer of Add() is a busy worker.
 */
void Worker::Add(const char* l)
{
    //fprintf(stderr,"Add() worker %d at %x\n", this->myId, this);
    //this->linelock.lock();
    this->lines.push_back(l);
    //this->linelock.unlock();
}

/*
 * The main loop of the thread. Don't call this directly.
 */
void Worker::Loop()
{

    // wrappers for mutex since we need to release locks before the scope
    // disappears
    std::unique_lock<std::mutex> rlw(this->runlock);
    while(1)
    {
	this->rungate.wait(rlw);
	this->linelock.lock();
	for ( std::string & s : this->lines )
	{
	    for ( RexItem* ri : this->rl )
	    {
		if (ri->DoMatch(s.c_str()) )
		{
		    broker::vector captures;
		    broker::vector ov;
		    std::string groupname;
		    std::map<std::string, int> argn2i;
		    int argno = 0;
		    int groupno = 1;

		    // populate ov with loginfo first
		    // just the start_time and hostname
		    if ( this->loginfo_forwarded->DoMatch(s.c_str()) )
		    {
			captures.resize(ri->GetNumCaptureGroups() + this->loginfo_forwarded->GetNumCaptureGroups());
			groupno = 1;
			for ( auto it = this->loginfo_forwarded->begin(); it != this->loginfo_forwarded->end(); it++ )
			{
			    this->loginfo_forwarded->GetGroupName(&groupname,groupno);
			    if ( groupname.compare("start_time") == 0 )
			    {
				fprintf(stderr,"Parsing start_time\n");
				captures[argno] = this->tp.Parse(it->c_str());
			    } else
			    {
				captures[argno] = it->c_str();
			    }
			    argn2i[groupname] = argno;
			    fprintf(stderr,"Map argn2i[%s] = %d (%s)\n", groupname.c_str(), argno, it->c_str());
			    ++groupno;
			    ++argno;
			}
		    } else if ( this->loginfo->DoMatch(s.c_str()) )
		    {
			captures.resize(ri->GetNumCaptureGroups() + this->loginfo->GetNumCaptureGroups());
			groupno = 1;
			for ( auto it = this->loginfo->begin(); it != this->loginfo->end(); it++ )
			{
			    this->loginfo->GetGroupName(&groupname,groupno);
			    if ( groupname.compare("start_time") == 0 )
			    {
				fprintf(stderr,"Parsing start_time\n");
				captures[argno] = this->tp.Parse(it->c_str());
			    } else
			    {
				captures[argno] = it->c_str();
			    }
			    argn2i[groupname] = argno;
			    fprintf(stderr,"Map argn2i[%s] = %d (%s)\n", groupname.c_str(), argno, it->c_str());

			    ++groupno;
			    ++argno;
			}
		    }

		    groupno = 1;

		    for ( auto  it = ri->begin(); it != ri->end(); it++ )
		    {
			ri->GetGroupName(&groupname, groupno);
			captures[argno]  = it->c_str();
			argn2i[groupname] = argno;
			fprintf(stderr,"Map argn2i[%s] = %d (%s)\n", groupname.c_str(), argno, it->c_str());

			++argno;
			++groupno;
		    }

		    // just testing!
		    groupno = 0;
		    ov.resize(2);
		    ov[0] = captures.at( argn2i["start_time"] );
		    ov[1] = captures.at( argn2i["hostname"] );
		    broker::bro::Event e("blar", ov);
		    //broker::bro::Event e(*ri->GetName(), ov);
		    ep->publish("/topic/test", e);
		}
	    }
	    //fprintf(stderr,"Got %s", s.c_str());
	}
	this->lines.clear();
	this->linelock.unlock();
	if ( !this->keepGoing.load(std::memory_order_relaxed) )
	{
	    //fprintf(stderr,"Bye! worker %d (%x)\n", this->myId, this->myThread);
	    this->workerPool->DelWorker(this->myId);
	    return;
	}
	this->workerPool->SetIdle(this->myId);
    }
}


/*
 * Call this to process whatever is in the input queue and exit
 */
void Worker::Drain()
{
    //fprintf(stderr, "Worker::Drain %d (%x)\n", this->myId, this->myThread);
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
