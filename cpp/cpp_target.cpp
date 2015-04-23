/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "cpp_target.hpp"
#include "cpp_from_cloog.hpp"
#include "cpp_from_polyhedral.hpp"
#include "../utility/cpp-gen.hpp"

using namespace std;

namespace stream {
namespace cpp_gen {

variable_decl_ptr variable_for(const semantic::type_ptr & t, const string & name)
{
    switch(t->get_tag())
    {
    case semantic::type::boolean:
        return make_shared<variable_decl>(make_shared<basic_type>("bool"), name);
    case semantic::type::integer_num:
        return make_shared<variable_decl>(make_shared<basic_type>("int"), name);
    case semantic::type::real_num:
        return make_shared<variable_decl>(make_shared<basic_type>("double"), name);
    case semantic::type::stream:
    {
        auto & stream = t->as<semantic::stream>();

        auto elem_type = type_for(stream.element_type);

        vector<int> size = stream.size;
        // FIXME:
        for(int & dim : size)
            if (dim == semantic::stream::infinite)
                dim = 0;

        return make_shared<array_decl>(elem_type, name, size);
    }
    default:
        throw error("Unexpected type.");
    }
}

func_sig_ptr signature_for(const string & name, const vector<semantic::type_ptr> & args)
{
    auto sig = new func_signature;
    sig->type = make_shared<basic_type>("void");
    sig->name = name;

    for (unsigned int input_idx = 0; input_idx < args.size(); ++input_idx)
    {
        ostringstream name;
        name << "in";
        name << input_idx;

        auto param = variable_for(args[input_idx], name.str());

        sig->parameters.push_back(param);
    }

    return func_sig_ptr(sig);
}

void generate(const string & name,
              const vector<semantic::type_ptr> & args,
              const vector<polyhedral::statement*> & poly_model,
              clast_stmt *finite_schedule,
              clast_stmt *periodic_schedule,
              std::ostream & stream)
{
    module m;
    builder b(&m);
    cpp_from_cloog cloog(&b);
    cpp_from_polyhedral poly(poly_model);

    auto stmt_func = [&]
            ( const string & name,
            const vector<expression_ptr> & index,
            builder * ctx)
    {
        poly.generate_statement(name, index, ctx);
    };

    cloog.set_stmt_func(stmt_func);

    if (finite_schedule)
    {
        auto sig = signature_for(name + "_finite", args);
        b.set_current_function(sig.get());

        auto func = make_shared<func_def>(sig);

        b.push(&func->body.statements);

        cloog.generate(finite_schedule);

        b.pop();

        m.members.push_back(func);
    }

    if (periodic_schedule)
    {
        auto sig = signature_for(name + "_period", args);
        b.set_current_function(sig.get());

        auto func = make_shared<func_def>(sig);

        b.push(&func->body.statements);

        cloog.generate(periodic_schedule);

        b.pop();

        m.members.push_back(func);
    }

    cpp_gen::state gen_state;

    m.generate(gen_state, stream);
}

}
}
