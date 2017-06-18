#include "error.hpp"
#include "../common/module.hpp"

#include <list>

using namespace std;

namespace stream {

void print(source_error::kind kind, const source_error & e)
{
    list<code_location> reverse_locations;
    auto trace_copy = e.trace;
    while(!trace_copy.empty())
    {
        auto location = trace_copy.top();
        if (!location.module)
            break;
        reverse_locations.push_front(location);
        trace_copy.pop();
    }
    for (auto & location : reverse_locations)
        cerr << ".. From " << location << endl;
    switch(kind)
    {
    case source_error::critical:
        cerr << "** ERROR: ";
        break;
    default:
        cerr << "** WARNING: ";
    }

    cerr << e.location << ": " << e.what() << endl;
    if (e.location.module)
        print_code_range(cerr, e.location.module->source, e.location.range);
}

}
