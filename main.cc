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

    Worker w(0,&p);
    Worker w2(1,&p);

    std::thread* th;
    th = new std::thread(Worker::RunWrap,&w);

    std::thread* th2;
    th2 = new std::thread(Worker::RunWrap,&w2);

    char linebuf[LINESIZE];

    Worker* v = p.GetIdleWorker();
    int linecount;

    while ( fgets(linebuf, LINESIZE, stdin) != NULL )
    {
	if ( linecount > 10 )
	{
	    linecount = 0;
	    v->Release();
	    v = p.GetIdleWorker();
	    fprintf(stderr,"Feeding worker %d\n",v->GetId());
	}

	    v->Add(linebuf);
	    linecount++;
	//for ( rexlist::iterator it = r.begin(); it != r.end(); ++it )
	//{
	//    (*it)->DoMatch(linebuf);
	//}
    }

    w.Drain();
    w2.Drain();

    th->join();
    th2->join();
    return 0;
}

