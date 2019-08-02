#ifndef __EVPARSE_HH_INCLUDED_
#define __EVPARSE_HH_INCLUDED_

#include <re2/re2.h>
#include <map>
#include <vector>
#include <string>
#include "rexitem.h"
#include <getopt.h>
#include <tuple>

/*
 * defs and other stuff for evparse
 */

// max line size (it's probably bigger than it ever needs to be)
#define LINESIZE 4194304

// each thread has a set of regexes to iterate over
// and check a given line against.
// this is that set.
typedef std::vector<RexItem*> RexList;

// to build a RexList, each thread needs to know the regex names, body, and
// order to present capture groups.  things got complicated.
// this describes a single regex.
typedef std::tuple<std::string, std::string, std::vector<std::string>> RexConfig;

// store a list of configs here
typedef std::vector<RexConfig> RexManifest;

// command-line args are described here for getopt
static struct option cli_opts[] = 
{
    {"configfile", required_argument, 0, 'f'},
};
#endif
