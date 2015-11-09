#ifndef STREAM_LANG_PARSING_DRIVER_INCLUDED
#define STREAM_LANG_PARSING_DRIVER_INCLUDED

#include "scanner.hpp"
#include "parser.hpp"
#include "../common/ast.hpp"

#include <iostream>

namespace stream {
namespace parsing {

class driver
{
    friend class parser;
    friend class scanner;

    class scanner scanner;
    class parser parser;
    std::istream m_input;

    ast::node_ptr m_ast;

public:
    driver(std::istream & in, std::ostream & out):
        scanner(*this, in, out),
        parser(*this),
        m_input(in.rdbuf())
    {
    }

    int parse()
    {
        return parser.parse();
    }

    ast::node_ptr ast()
    {
        return m_ast;
    }

    void error(const parser::location_type& l, const std::string& m);
};

}
}

#endif // STREAM_LANG_PARSING_DRIVER_INCLUDED
