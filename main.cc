#include <stdio.h>
#include <iostream>
#include "brokersaurusrex.h"
#include "rexitem.h"
#include "worker.h"
#include "pool.h"
#include <broker/broker.hh>
#include <broker/bro.hh>
#include <getopt.h>
#include <libconfig.h++>

void usage(char* progname)
{
    fprintf(stderr, "Usage: %s --configfile=<config file in libconf format>\n", progname);
    exit(1);
}

int main(int argc, char* argv[])
{
    RexItem* ri;
    RexManifest rm;
    broker::endpoint ep;
    std::string configfile;

    // figure out the configuration, since we don't like to recompile every time
    // just a config file for now, everything goes in there.
    int optidx = 0;
    int optc = 0;
    while ( (optc = getopt_long(argc, argv, "f:", cli_opts, &optidx)) != -1 )
    {
	switch(optc)
	{
	    case 'f':
		configfile = optarg;
		break;
	}
    }

    if ( configfile.empty() )
    {
	usage(argv[0]);
    }

    // now suck in everything else we need from the config file

    libconfig::Config conf;
    
    try
    {
	conf.readFile(configfile.c_str());
    }
    catch ( const libconfig::FileIOException &e )
    {
	fprintf(stderr, "I/O on file: %s\n", configfile);
	return(-1);
    }
    catch ( const libconfig::ParseException &e )
    {
	fprintf(stderr, "Parse error %s (line %d): %s\n", e.getFile(), e.getLine(), e.getError());
	return(-2);
    }

    // we'll try to defer exit so as to discover as many errors as possible
    bool oopsFound = false;

    // peer IP
    std::string peer = "";
    if ( ! conf.exists("peer") )
    {
	fprintf(stderr, "Error %s: Missing setting peer\n", configfile.c_str());
	oopsFound = true;
    }
    else
    {
	peer = conf.lookup("peer").c_str();
    }

    // peer port
    int peerport = 0;
    if ( ! conf.exists("peerport") )
    {
	fprintf(stderr, "Error %s: Missing setting peerport\n", configfile.c_str());
	oopsFound = true;
    }
    else
    {
	peerport = conf.lookup("peerport");
    }

    // topic
    std::string topic = "";
    if ( ! conf.exists("topic") )
    {
	fprintf(stderr, "Error %s: Missing setting topic\n", configfile.c_str());
	oopsFound = true;
    }
    else
    {
	topic = conf.lookup("topic").c_str();
    }

    // events
    if ( ! conf.exists("events") )
    {
	fprintf(stderr, "Error %s: Missing setting events\n", configfile.c_str());
	oopsFound = true;
    }

    // check for omissions here.
    if ( oopsFound )
    {
	fprintf(stderr, "Errors found. Cannot continue\n");
	return(1);
    }

    // events contd.
    libconfig::Setting& eventroot = conf.lookup("events");
    int eventcount = eventroot.getLength();
    if ( eventcount < 1 )
    {
	fprintf(stderr, "Found events setting but zero events. This config file does nothing!\n");
	return(2);
    }

    for ( int i = 0; i < eventcount; i++ )
    {
	const libconfig::Setting& s = eventroot[i];
	std::vector<std::string> arglist;

	// event name
	const std::string name = s.getName();

	// check for missing sub-settings
	if ( ! s.exists("arglist") )
	{
	    fprintf(stderr, "Event %s missing setting arglist\n", name.c_str());
	    oopsFound = true;
	}
	if ( ! s.exists("regex") )
	{
	    fprintf(stderr, "Event %s missing setting regex\n", name.c_str());
	    oopsFound = true;
	}
	if ( oopsFound )
	{
	    fprintf(stderr, "Errors round with event %s.\n  Refusing to continue to prevent illusion of doing something useful. Sorry!\n", name.c_str());
	    return(3);
	}

	// ordered list of capture groups to put in broker event
	libconfig::Setting& argnode = s["arglist"];
	int argcount = argnode.getLength();

	// arguably argcount can be zero. maybe they just want to know
	// something happened, not any details

	for ( int j = 0; j < argcount; j++ )
	{
	    arglist.push_back(argnode[j].c_str());
	}

	// and the regex
	const std::string regex = s["regex"];

	// roll them up
	RexConfig rc = std::make_tuple(name,regex,arglist);
	rm.push_back(rc);
	//rm[name] = regex;
    }


    // connect to peer
    auto epss = ep.make_status_subscriber(true);
    ep.peer(peer.c_str(), peerport, std::chrono::seconds(2));
    auto epss_res = epss.get();
    auto sc = caf::get_if<broker::status>(&epss_res);

    if ( ! ( sc && sc->code() == broker::sc::peer_added) )
    {
	fprintf(stderr, "Could not connect to broker peer\n");
	return 1;
    }

    //fprintf(stderr, "Connected. Spew away!\n");

    Pool p;
    Worker* w;
    std::thread* th;

    for ( int widx = 0; widx < 4; widx++ )
    {
	w = new Worker(widx, &p, topic);
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

