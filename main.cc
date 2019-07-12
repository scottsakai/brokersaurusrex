#include <stdio.h>
#include <iostream>
#include "brokersaurusrex.h"
#include "rexitem.h"
#include "worker.h"
#include "pool.h"



int main(int argc, char* argv[])
{
    RexItem* ri;
    rexlist r;

    
    ri = new RexItem("ssh-success", 
      " sshd\\[(?P<pid>\\d+)\\]: Accepted (?P<authmethod>\\S+) for (?P<username>\\S+) from (?P<remaddr>\\S+) port (?P<remport>\\d+)");
    r.push_back(ri);


    ri = new RexItem("ssh-fingerprint",
      " sshd\\[(?P<pid>\\d+)\\]: Found matching (?P<keytype>\\S+) key: (?P<fingerprint>\\S+)");
    r.push_back(ri);

    Pool p;
    Worker* w;
    std::thread* th;

    for ( int widx = 0; widx < 10; widx++ )
    {
	w = new Worker(widx, &p);
	th = new std::thread(Worker::RunWrap, w);
	w->SetThread(th);
    }

    char linebuf[LINESIZE];

    w = p.GetIdleWorker();
    int linecount;

    while ( fgets(linebuf, LINESIZE, stdin) != NULL )
    {
	// only shove so many lines into the worker, then set it free
	// and pick another worker
	if ( linecount > 1000000 )
	{
	    linecount = 0;
	    w->Release();

	    // sometimes there are no idle workers...
	    while ( (w = p.GetIdleWorker()) == NULL )
	    {
		p.WaitForIdleWorker();
	    }
	}

	    w->Add(linebuf);
	    linecount++;
	//for ( rexlist::iterator it = r.begin(); it != r.end(); ++it )
	//{
	//    (*it)->DoMatch(linebuf);
	//}
    }
    // make sure the current worker finishes its workload if any
    w->Drain();

    // and clean up.
    p.Shutdown();
    return 0;
}

