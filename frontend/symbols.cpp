#include "symbols.hpp"
#include "types.hpp"

#include <stdexcept>
#include <cassert>
#include <vector>
#include <iostream>

using namespace std;

namespace stream {
namespace symbolic {

environment construct_environment( ast::node * root )
{
    using ast::list_node;
    using ast::leaf_node;
    using ast::node;

    if (root->type != ast::program)
        throw std::runtime_error("Root node is not a program.");

    environment env;

    list_node *statements = static_cast<list_node*>(root);
    for ( sp<node> & child : statements->elements )
    {


        assert(child->type == ast::statement);
        list_node *statement = static_cast<list_node*>(child.get());

        assert(statement->elements.size() == 3);

        // Get name

        node *id = statement->elements[0].get();
        assert(id);
        assert(id->type == ast::identifier);

        string func_name = static_cast<leaf_node<string>*>(id)->value;

        // Get parameters

        vector<string> param_names;

        node *params = statement->elements[1].get();
        if (params)
        {
            assert(params->type == ast::id_list);
            list_node *param_list = static_cast<list_node*>(params);
            for ( sp<node> & param : param_list->elements )
            {
                assert(param->type == ast::identifier);
                string param_name =
                        static_cast<leaf_node<string>*>(param.get())->value;
                param_names.push_back(param_name);
            }
        }

        sp<type::function> & f = env[func_name];
        if (f)
        {
            cerr << "[line " << child->line << "] ERROR: "
                 << "Name already declared in same scope: " << f->name
                 << endl;
        }
        else
        {
            f.reset(new type::function);
            f->name = func_name;
            f->parameters = std::move(param_names);

            cout << "[line " << child->line << "] "
                 << "Adding top-level declaration: " << f->name
                 << endl;
        }
    }

    return std::move(env);
}


} // namespace symbolic
} // namespace stream
