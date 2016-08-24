#include "module_parser.hpp"
#include "driver.hpp"
#include "../common/error.hpp"
#include "../common/ast_printer.hpp"
#include "../utility/debug.hpp"

#include <sstream>

using namespace std;

namespace stream {

module * module_parser::parse(const module_source & source, istream & text_stream)
{
    if (verbose<module_parser>::enabled())
        cout << "Parsing " << source.path << endl;

    ast::node_ptr ast;

    {
        stream::parsing::driver parser(text_stream, cout, source);

        int error = parser.parse();

        if (error)
            throw parser_error();

        ast = parser.ast();

        if (verbose<ast::output>::enabled())
        {
            cout << endl << "## AST for file " << source.path << " ##" << endl;
            ast::printer p;
            p.print(ast.get());
            cout << endl;
        }
    }

    auto & elems = ast->as_list()->elements;

    string mod_name;
    if (elems[0])
        mod_name = elems[0]->as_leaf<string>()->value;
    else
        mod_name = "m";

#if 0
    if (m_named_modules.find(mod_name) != m_named_modules.end())
    {
        ostringstream msg;
        msg << "Module " << mod_name << " is already imported.";
        throw io_error(msg.str());
    }
#endif

    if (verbose<module_parser>::enabled())
        cout << "Module name = " << mod_name << endl;

    auto mod = new module;
    mod->source = source;
    mod->name = mod_name;
    mod->ast = ast;
    m_named_modules.emplace(mod_name, mod);

    unordered_map<string, module*> imported_mods;

    auto imports_node = elems[1];
    if (imports_node)
    {
        for (auto & import_node : imports_node->as_list()->elements)
        {
            auto import_decl = parse_import(mod, import_node);
            imported_mods.insert(import_decl);
        }
    }

    mod->imports = imported_mods;

    m_ordered_modules.push_back(mod);

    if (verbose<module_parser>::enabled())
        cout << "Done parsing " << source.path << endl;

    return mod;
}

pair<string, module*> module_parser::parse_import
(const module * importer, const ast::node_ptr & import_node)
{
    code_location import_location(importer, import_node->location);

    auto & elems = import_node->as_list()->elements;
    auto import_name = elems[0]->as_leaf<string>()->value;

    string import_alias;
    if (elems[1])
        import_alias = elems[1]->as_leaf<string>()->value;
    else
        import_alias = import_name;

    if (verbose<module_parser>::enabled())
        cout << "Importing: " << import_name << " as " << import_alias << endl;

    auto imported_mod_it = m_named_modules.find(import_name);
    if (imported_mod_it == m_named_modules.end())
    {
        string import_filename = import_name + ".stream";
        module_source import_source;
        ifstream text;

        import_source.dir = importer->source.dir;

        if (!import_source.dir.empty())
            import_source.path += import_source.dir + '/';
        import_source.path += import_filename;

        if (verbose<module_parser>::enabled())
            cout << "Trying " << import_source.path << endl;

        text.open(import_source.path);
        if (!text.is_open())
        {
            for (auto & idir : m_import_dirs)
            {
                import_source.dir = idir;
                import_source.path = idir + "/" + import_filename;
                if (verbose<module_parser>::enabled())
                    cout << "Trying " << import_source.path << endl;
                text.open(import_source.path);
                if (text.is_open())
                    break;
            }
        }

        if (!text.is_open())
        {
            ostringstream msg;
            msg << "** ERROR: " << import_location << ": "
                << "Can not find imported module " << import_name << ".";
            throw io_error(msg.str());
        }

        parse(import_source, text);
    }
    else
    {
        if (verbose<module_parser>::enabled())
            cout << "Already parsed." << endl;
    }

    imported_mod_it = m_named_modules.find(import_name);
    if (imported_mod_it == m_named_modules.end())
    {
        ostringstream msg;
        msg << "** ERROR: " << import_location << ": "
            << "Failed to import module " << import_name << ".";
        throw io_error(msg.str());
    }

    return make_pair(import_alias, imported_mod_it->second);
}

}
