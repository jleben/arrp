#ifndef STREAM_POLYHEDRAL_PRINTER_INCLUDED
#define STREAM_POLYHEDRAL_PRINTER_INCLUDED

#include "model.hpp"

#include <iostream>
#include <string>

namespace stream {
namespace polyhedral {

using std::ostream;
using std::string;

class printer
{
public:
    printer();
    void print(const expression *expr, ostream &);
    void print(const statement *stmt, ostream & );
    void indent() { ++level; }
    void unindent() { --level; }
private:
    string indentation() { return string(level * 2, ' '); }
    int level;

};

}
}
#endif // STREAM_POLYHEDRAL_PRINTER_INCLUDED
