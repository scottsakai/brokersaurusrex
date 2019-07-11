#ifndef __EVPARSE_HH_INCLUDED_
#define __EVPARSE_HH_INCLUDED_

#include <re2/re2.h>
#include <map>
#include <vector>
#include <string>
#include "rexitem.h"

/*
 * defs and other stuff for evparse
 */

// max line size (it's probably bigger than it ever needs to be)
#define LINESIZE 4194304

// each thread has a set of regexes to iterate over
// and check a given line against.
// this is that set.
typedef std::vector<RexItem*> rexlist;



#endif
