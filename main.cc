#include <stdio.h>
#include <iostream>
#include "brokersaurusrex.h"
#include "rexitem.h"
#include "worker.h"
#include "pool.h"
#include <broker/broker.hh>
#include <broker/bro.hh>


int main(int argc, char* argv[])
{
    RexItem* ri;
    RexManifest rm;
    broker::endpoint ep;


    // connect to peer
    auto epss = ep.make_status_subscriber(true);
    ep.peer("127.0.0.1", 9998, std::chrono::seconds(2));
    auto epss_res = epss.get();
    auto sc = caf::get_if<broker::status>(&epss_res);

    if ( ! ( sc && sc->code() == broker::sc::peer_added) )
    {
	fprintf(stderr, "Could not connect to broker peer\n");
	return 1;
    }

    fprintf(stderr, "Connected. Spew away!\n");

    rm["ssh_success"] = " sshd\\[(?P<pid>\\d+)\\]: Accepted (?P<authmethod>\\S+) for (?P<username>\\S+) from (?P<remaddr>\\S+) port (?P<remport>\\d+)";

    rm["ssh_fingerprint"] = " sshd\\[(?P<pid>\\d+)\\]: Found matching (?P<keytype>\\S+) key: (?P<fingerprint>\\S+)";

    Pool p;
    Worker* w;
    std::thread* th;

    for ( int widx = 0; widx < 4; widx++ )
    {
	w = new Worker(widx, &p);
	th = new std::thread(Worker::RunWrap, w);
	w->SetThread(th);
	w->SetEndpoint(&ep);
	w->Compile(&rm);
    }

    char linebuf[LINESIZE];

    w = p.GetIdleWorker();
    int linecount;

    // xinetd will make sure stuff gets flushed in a timely manner
    // but we'll still need to send off the work early if things are quiet.
    auto lastrelease = std::chrono::steady_clock::now();
    while ( fgets(linebuf, LINESIZE, stdin) != NULL )
    {
	auto now = std::chrono::steady_clock::now();
	// only shove so many lines into the worker, then set it free
	// and pick another worker
	std::chrono::duration<double> diff = now - lastrelease;
	if ( linecount > 1000000  || diff.count() > 1 )
	{
	    linecount = 0;
	    w->Release();
	    lastrelease = std::chrono::steady_clock::now();

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
    sleep(3);
    ep.unpeer("127.0.0.1", 9998);
    ep.shutdown();
    return 0;
}

