#pragma once

#include "testing.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>

namespace arrp {
namespace testing {

using std::vector;
using std::string;
using std::istream;

class test_parser
{
public:
    struct error : public std::exception
    {
        error() {}
        error(const string & r): m_reason(r) {}
        const char * what() const noexcept override { return m_reason.c_str(); }
    private:
        string m_reason;
    };

    void parse(istream & src);

    elem_type type() const { return m_type; }
    const vector<int> & size() const { return m_size; }
    const array & data() const { return m_data; }

private:
    void skip_space();
    void parse_sample();
    void parse_element();
    void parse_list();
    void parse_value();
    void store_value(const string & value_text, bool real);
    void increment_index();
    void expand_index();
    void contract_index();
    char next_char();
    void pop_char();

    elem_type m_type;
    vector<int> m_size;
    array m_data;

    istream * m_src;
    char m_c = 0;
    vector<int> m_index;
};

}
}
