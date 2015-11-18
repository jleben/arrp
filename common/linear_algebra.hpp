#ifndef STREAM_LANG_FUNCTIONAL_LINEAR_ALGEBRA_INCLUDED
#define STREAM_LANG_FUNCTIONAL_LINEAR_ALGEBRA_INCLUDED

#include "functional_model.hpp"

#include <vector>
#include <utility>

namespace stream {
namespace functional {

using std::vector;
using std::pair;

class linexpr
{
    vector<pair<var_ptr,int>> terms;

public:
    typedef pair<var_ptr,int> term_type;
    typedef vector<term_type> term_list_type;
    typedef term_list_type::iterator iterator;
    typedef term_list_type::const_iterator const_iterator;

    class invalid {};

    linexpr() {}

    linexpr(std::initializer_list<term_type> list):
        terms(list) {}

    linexpr(int s)
    {
        terms.emplace_back(var_ptr(),s);
    }

    linexpr(const var_ptr & v, int s = 1)
    {
        terms.emplace_back(v,s);
    }

    friend linexpr operator* (const var_ptr & v, int s)
    {
        linexpr e;
        e.terms.emplace_back(v,s);
        return e;
    }

    linexpr operator+ (const linexpr & other)
    {
        linexpr e(*this);
        for (const auto & other_term : other.terms)
        {
            bool done = false;
            for(auto & my_term : e.terms)
            {
                if (my_term.first == other_term.first)
                {
                    my_term.second += other_term.second;
                    done = true;
                }
            }
            if (!done)
            {
                e.terms.push_back(other_term);
            }
        }
        return e;
    }

    linexpr operator+ (const term_type & other_term)
    {
        linexpr e(*this);

        bool done = false;
        for(auto & my_term : e.terms)
        {
            if (my_term.first == other_term.first)
            {
                my_term.second += other_term.second;
                done = true;
            }
        }
        if (!done)
        {
            e.terms.push_back(other_term);
        }

        return e;
    }

    linexpr operator-()
    {
        linexpr e(*this);
        for (auto & term : e.terms)
            term.second = - term.second;
        return e;
    }

    linexpr operator* (int s)
    {
        linexpr e(*this);
        for (auto & term : e)
            term.second *= s;
        return e;
    }

    bool is_constant()
    {
        return terms.size() == 1 && !terms[0].first;
    }

    int constant()
    {
        if (!is_constant())
            throw invalid();
        return terms[0].second;
    }

    int term_count() const { return terms.size(); }

    int var_count() const
    {
        int c = 0;
        for (const auto & term : terms)
            if (term.first)
                ++c;
        return c;
    }

    iterator begin() { return terms.begin(); }
    iterator end() { return terms.end(); }
    const_iterator begin() const { return terms.begin(); }
    const_iterator end() const { return terms.end(); }
};

}
}

#endif // STREAM_LANG_FUNCTIONAL_LINEAR_ALGEBRA_INCLUDED
