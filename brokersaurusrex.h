#ifndef __EVPARSE_HH_INCLUDED_
#define __EVPARSE_HH_INCLUDED_

#include <re2/re2.h>
#include <map>
#include <vector>
#include <string>
#include "rexitem.h"
#include <getopt.h>

/*
 * defs and other stuff for evparse
 */

// max line size (it's probably bigger than it ever needs to be)
#define LINESIZE 4194304

// each thread has a set of regexes to iterate over
// and check a given line against.
// this is that set.
typedef std::vector<RexItem*> RexList;

// to build a RexList, each thread needs to know the regex names and body.
// store them here.
typedef std::map<std::string, std::string> RexManifest;

// command-line args are described here for getopt
static struct option cli_opts[] = 
{
    {"configfile", required_argument, 0, 'f'},
};
#endif
