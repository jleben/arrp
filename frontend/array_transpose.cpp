#include "array_transpose.hpp"
#include "error.hpp"
#include "../utility/debug.hpp"

#include <cassert>

using namespace std;

namespace stream {
namespace functional {

void array_transposer::process(const unordered_set<id_ptr> & ids)
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

void array_transposer::transpose_arrays(const unordered_set<id_ptr> & ids)
{
    for (auto & id : ids)
        transpose_array(id);
}

void array_transposer::transpose_array(const id_ptr & id)
{
    array_type * ar_type = nullptr;

    ar_type = id->expr->type->array();

    if (!ar_type)
    {
        if (verbose<array_transposer>::enabled())
            cout << "== Not an array: " << id << endl;
        return;
    }

    if (verbose<array_transposer>::enabled())
        cout << "== Transposing array: " << id << endl;

    list<int> order;

    bool transposed =
            transpose_order(id, ar_type->size, order);

    if (transposed)
    {
        if (auto ar = dynamic_pointer_cast<array>(id->expr.expr))
        {
            vector<array_var_ptr> ordered_vars;
            assert(ar->vars.size() <= order.size());

            for (int i : order)
            {
                if (ordered_vars.size() == ar->vars.size())
                    break;

                // FIXME:
                // Make sure implicit array variables are not referenced
                // in the array expression.
                // Ideally, these array dimensions would be removed.
                if (i >= ar->vars.size())
                    throw error("Can not reorder implicit array dimension.");

                ordered_vars.push_back(ar->vars[i]);
            }

            ar->vars = ordered_vars;
        }
        else if (!(dynamic_pointer_cast<external>(id->expr.expr)))
        {
            ostringstream msg;
            msg << "Array transposition: Unexpected expression type for id " << id->name;
            throw error(msg.str());
        }

        array_size_vec ordered_size;
        for (int i : order)
            ordered_size.push_back(ar_type->size[i]);
        ar_type->size = ordered_size;

        vector<int> order_vector(order.begin(), order.end());
        m_transpositions[id] = order_vector;
    }
}

bool array_transposer::transpose_order
(id_ptr id, const array_size_vec & size, list<int> & order)
{
    bool has_streaming_dimension = false;

    for (int i = 0; i < (int) size.size(); ++i)
    {
        // The range is either a constant integer, or none.
        if (size[i] >= 0)
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

    return has_streaming_dimension;
}

void array_transposer::transpose_accesses(const unordered_set<id_ptr> & ids)
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

void array_transposer::visit_array(const shared_ptr<array> & arr)
{
    // Don't visit array scope.

    visit(arr->expr);
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

    if (verbose<array_transposer>::enabled())
    {
        cout << ".. Transposition for access to id " << id << " = ";
        for (int dim : order)
            cout << " " << dim;
        cout << endl;
    }

    vector<expr_slot> transposed_args;

    assert(app->args.size() <= order.size());

    while(transposed_args.size() < app->args.size())
    {
        int dst = transposed_args.size();
        int src = order[dst];
        // FIXME:
        if (src >= app->args.size())
            throw error("Can not reorder implicit array arguments.");
        transposed_args.emplace_back(app->args[src]);
    }

    app->args = transposed_args;
}


}
}
