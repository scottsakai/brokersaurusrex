#include "rexitem.h"
#include <broker/bro.hh>

RexItem::RexItem(const char* name, const char* regexdef)
{
    // nobody else really cares what the name is but us.
    this->name = name;

    // start with the re2 object
    this->re = new RE2(regexdef);

    // now the places to stuff our capture results
    this->numgroups = this->re->NumberOfCapturingGroups();
    this->re2args.resize(this->numgroups);
    this->matches.resize(this->numgroups);

    for ( int i = 0; i < this->numgroups; ++i )
    {
	this->re2args[i] =  new RE2::Arg(&(this->matches[i]));
    }

}

/*
 * Match line against stored re2. Fire off a bro event if matched.
 */
bool RexItem::DoMatch(const char* line, broker::endpoint* ep)
{
    if ( RE2::PartialMatchN(line, *this->re, 
      &(this->re2args.front()), 
      this->numgroups) )
    {
	broker::vector ov;
	ov.resize(this->numgroups);
	int argno = 0;
	for ( auto & it : this->matches)
	{
	    //fprintf(stderr, "Adding %s at idx %d\n", it.c_str(), argno);
	    ov[argno++] = it.c_str();
	}
	broker::bro::Event e(this->name, ov);
	ep->publish("/topic/test", e);
	//printf("Event %s %s\n", this->name.c_str(),
	//    this->matches[1].c_str());
	return true;
    }

    return false;
}


