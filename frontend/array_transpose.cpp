#include "array_transpose.hpp"
#include "error.hpp"
#include "../utility/debug.hpp"

#include <cassert>

using namespace std;

namespace stream {
namespace functional {

void array_transposer::process(unordered_set<id_ptr> & ids)
{
    try
    {
        transpose_arrays(ids);
        transpose_accesses(ids);
    }
    catch (...)
    {
        m_transpositions.clear();
        m_current_id = nullptr;
        throw;
    }

    m_transpositions.clear();
    m_current_id = nullptr;
}

void array_transposer::transpose_arrays(unordered_set<id_ptr> & ids)
{
    for (auto & id : ids)
        transpose_array(id);
}

void array_transposer::transpose_array(const id_ptr & id)
{
    auto ar = dynamic_pointer_cast<array>(id->expr.expr);
    if (!ar)
    {
        if (verbose<array_transposer>::enabled())
            cout << "== Not an array: " << id << endl;
        return;
    }

    if (verbose<array_transposer>::enabled())
        cout << "== Transposing array: " << id << endl;

    list<int> order;
    bool has_streaming_dimension = false;

    for (int i = 0; i < (int)ar->vars.size(); ++i)
    {
        // The range is either a constant integer, or none.
        auto & var = ar->vars[i];
        if (var->range)
        {
            order.push_back(i);
        }
        else
        {
            if (has_streaming_dimension)
            {
                ostringstream msg;
                msg << "Array " << id->name
                    << " has more than 1 unbounded dimension.";
                throw source_error(msg.str(), id->location);
            }
            has_streaming_dimension = true;
            order.push_front(i);
            if (verbose<array_transposer>::enabled())
                cout << ".. Streaming dimension = " << i << endl;
        }
    }

    if (has_streaming_dimension)
    {
        vector<array_var_ptr> ordered_vars;
        for (int i : order)
            ordered_vars.push_back(ar->vars[i]);
        ar->vars = ordered_vars;

        vector<int> order_vector(order.begin(), order.end());
        m_transpositions[id] = order_vector;
    }
}

void array_transposer::transpose_accesses(unordered_set<id_ptr> & ids)
{
    for (auto & id : ids)
        transpose_access(id);
}

void array_transposer::transpose_access(const id_ptr & id)
{
    if (verbose<array_transposer>::enabled())
    {
        cout << "== Transposing accesses in expression of " << id << endl;
    }

    m_current_id = id;
    visit(id->expr);
}

void array_transposer::visit_array_app(const shared_ptr<array_app> & app)
{
    id_ptr id;

    if (auto ref = dynamic_pointer_cast<reference>(app->object.expr))
    {
        id = dynamic_pointer_cast<identifier>(ref->var);
    }
    else if (auto ref = dynamic_pointer_cast<array_self_ref>(app->object.expr))
    {
        id = m_current_id;
    }

    assert(id);

    auto transposition = m_transpositions.find(id);
    if (transposition == m_transpositions.end())
    {
        if (verbose<array_transposer>::enabled())
        {
            cout << ".. No transposition for access to id " << id << endl;
        }
        return;
    }

    auto & order = transposition->second;

    assert(order.size() == app->args.size());

    if (verbose<array_transposer>::enabled())
    {
        cout << ".. Transposition for access to id " << id << " = ";
        for (int dim : order)
            cout << " " << dim;
        cout << endl;
    }

    vector<expr_slot> transposed_args;

    for (int i = 0; i < (int)order.size(); ++i)
    {
        int dim = order[i];
        transposed_args.emplace_back(app->args[dim]);
    }

    app->args = transposed_args;
}


}
}
