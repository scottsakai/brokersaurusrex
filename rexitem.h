#ifndef _rexitem_h_included_
#define _rexitem_h_included_

#include <string>
#include <vector>
#include <re2/re2.h>
#include <broker/broker.hh>

// re2argset is a set of RE2::Arg pointers for a specific regex's capture
// groups.  Do not share this between threads.
typedef std::vector<const RE2::Arg*> re2argset;

// captureset is a set of strings for a specific regex's capture groups.
// Do not share this between threads.
typedef std::vector<std::string> captureset;



/*
 * rexitem class: represents a single RE2 regex and the dynamically-allocated 
 * structures to store its capture groups.
 */
class RexItem
{
    private:
    re2argset re2args;
    captureset matches;
    RE2* re;
    int numgroups;
    std::string name;

    public:
    RexItem(const char* name, const char* regexdef);

    /* Attempt to match line
     * Will process and emit a bro event if matched
     * Returns TRUE on match
     */
    bool DoMatch(const char* line, broker::endpoint* ep);

}; // end of RexItem class




#endif
