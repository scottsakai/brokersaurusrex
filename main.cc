#include <stdio.h>
#include <iostream>
#include "brokersaurusrex.h"
#include "rexitem.h"




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

    char linebuf[LINESIZE];
    while ( fgets(linebuf, LINESIZE, stdin) != NULL )
    {
        //printf("Got %s\n", linebuf);
	for ( rexlist::iterator it = r.begin(); it != r.end(); ++it )
	{
	    (*it)->DoMatch(linebuf);
	}
    }


    return 0;
}

