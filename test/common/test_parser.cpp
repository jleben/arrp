#include "test_parser.hpp"

#include <string>
#include <sstream>

using namespace std;

namespace arrp {
namespace testing {

void test_parser::parse(istream & src)
{
    string line;

    auto pos = string::npos;

    while(std::getline(src, line))
    {
        pos = line.find("##?");
        if (pos != string::npos)
            break;
    }

    if (pos == string::npos)
        return;

    pos += 3;

    // Parse output size

    m_size.clear();

    auto parse_dim_size = [this](const string & str)
    {
        if (str == "~")
        {
            m_size.push_back(-1);
        }
        else
        {
            string::size_type consumed;
            int s = stoi(str, &consumed);
            if (consumed != str.size())
                throw error("Invalid array size value.");
            m_size.push_back(s);
        }
    };

    {
        auto size_decl_pos = line.find('[', pos);
        if (size_decl_pos != string::npos)
        {
            pos = size_decl_pos;
            auto start = pos + 1;
            auto end = start;
            while ((end = line.find(',', start)) != string::npos)
            {
                parse_dim_size(line.substr(start, end-start));
                start = end + 1;
            }

            end = line.find(']', start);
            if (end == string::npos)
                throw error("Could not parse array size.");
            parse_dim_size(line.substr(start, end-start));

            pos = end + 1;
        }
    }

    // Parse elem type

    {
        string type_str;

        istringstream stream(line.substr(pos));
        stream >> type_str;

        if (type_str.empty())
            throw error("Could not parse data type.");

        if (type_str == "bool")
            m_type = bool_type;
        else if (type_str == "int")
            m_type = int_type;
        else if (type_str == "real32")
            m_type = float_type;
        else if (type_str == "real64")
            m_type = double_type;
        else
            throw error("Invalid type name: " + type_str);

        m_data.set_type(m_type);
    }

    // Parse expected data

    {
        auto data_size = m_size;

        if (data_size.size())
        {
            data_size[0] = 0;
            m_index.push_back(0);
        }

        m_data.resize(data_size);
    }

    bool has_data = false;

    while(std::getline(src, line))
    {
        pos = line.find("##?");
        if (pos == string::npos)
            continue;
        pos += 3;

        if (has_data && m_size.empty())
            throw error("Multiple elements given, but data is scalar.");

        istringstream stream(line.substr(pos));
        m_src = &stream;

        if (m_size.size())
            m_data.extend(1);

        parse_element();

        if (m_size.size())
            increment_index();

        has_data = true;
    }

    if (m_size.size())
    {
        if (m_size[0] >= 0 && m_data.count() != m_size[0])
        {
            throw error("Wrong number of elements in dimension 0.");
        }
    }
}

void test_parser::skip_space()
{
    while(next_char() == ' ')
        pop_char();
}

void test_parser::parse_element()
{
    skip_space();
    char c = next_char();
    if (c == '(')
        parse_list();
    else
        parse_value();
    skip_space();
}

void test_parser::parse_list()
{
    char c;

    c = next_char();
    if (c != '(')
        throw error(string("Expected '(' but got '") + c + "'.");
    pop_char();

    expand_index();

    int dim = m_index.size() - 1;

    if (m_index.size() > m_size.size())
        throw error("Too many dimensions.");

    while(true)
    {
        parse_element();
        c = next_char();
        if (c == ',')
        {
            pop_char();
            increment_index();
            if (m_index.back() >= m_size[dim])
                throw error("Too many elements in dimension " + to_string(dim) + ".");
        }
        else
            break;
    }

    if (c != ')')
        throw error(string("Expected ')' but got '") + c + "'.");
    pop_char();

    if (m_index.back() < m_size[dim]-1)
        throw error("Too few elements in dimension " + to_string(dim) + ".");

    contract_index();
}

void test_parser::parse_value()
{
    if (m_index.size() != m_size.size())
        throw error("Value at wrong nesting level.");

    string text;
    char c;
    bool has_dot = false;

    c = next_char();

    if (c == '-' || c == '+')
    {
        text += c;
        pop_char();
        c = next_char();
    }

    if (!isdigit(c))
        throw error("Could not parse value.");

    while(true)
    {
        c = next_char();
        if (isdigit(c))
        {
            text += c;
        }
        else if (c == '.' && !has_dot)
        {
            has_dot = true;
            text += c;
        }
        else
            break;
        pop_char();
    }

    store_value(text, has_dot);
}

void test_parser::store_value(const string & text, bool real)
{
    element e;

    if (m_type == double_type || m_type == float_type)
    {
        string::size_type pos;
        double v = stod(text, &pos);
        if (pos != text.size())
            throw error("Could not parse value: " + text);

        if (m_type == double_type)
            e = element(v);
        else
            e = element(float(v));
    }
    else if (m_type == int_type)
    {
        string::size_type pos;
        int v = stoi(text, &pos);
        if (pos != text.size())
            throw error("Could not parse value: " + text);

        e = element(v);
    }
    else
    {
        throw error("Unsupported value type.");
    }

    m_data(m_index) = e;
}

void test_parser::increment_index()
{
    if (m_index.empty())
        throw error();
    ++m_index.back();
}

void test_parser::expand_index()
{
    m_index.push_back(0);
}

void test_parser::contract_index()
{
    if (m_index.empty())
        throw error();
    m_index.pop_back();
}

char test_parser::next_char()
{
    if (!m_c && m_src)
        m_src->get(m_c);
    return m_c;
}

void test_parser::pop_char()
{
    m_c = 0;
}


}
}
