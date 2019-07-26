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
bool RexItem::DoMatch(const char* line)
{
    if ( RE2::PartialMatchN(line, *this->re, 
      &(this->re2args.front()), 
      this->numgroups) )
    {
	return true;
    }

    return false;
}


/* 
 * Retrieve a matched capture group by its name
 *
 * Returns empty string if no match.
 */
const std::string RexItem::operator[] (std::string k)
{
    int matchIndex = 0;
    auto n2i = this->re->NamedCapturingGroups();

    // no match -> empty string
    if ( n2i.find(k) == n2i.end() )
    {
	return "";
    }

    // try to get the index then
    matchIndex = n2i.at(k);

    // then the value
    return this->matches.at(matchIndex);
}


/*
 * Retrieve a matched capture group by its name
 *
 * Places result in dest, empty string if no match.
 *
 * Returns false if no match, true otherwise.
 */
bool RexItem::GetCapture(std::string * dest, std::string key)
{
    int matchIndex = 0;
    auto n2i = this->re->NamedCapturingGroups();

    // no match -> empty string
    if ( n2i.find(key) == n2i.end() )
    {
	*dest = "";
	return false;
    }

    // try to get the index then
    matchIndex = n2i.at(key);

    // then the value
    *dest = this->matches.at(matchIndex);
    return true;
}

/**
 * Convert a group offset to group name
 */
bool RexItem::GetGroupName(std::string * dest, int offset)
{
    auto i2n = this->re->CapturingGroupNames();

    // no match -> empty string
    if ( i2n.find(offset) == i2n.end() )
    {
	*dest = "";
	return false;
    }

    *dest = i2n.at(offset);
    return true;
}
