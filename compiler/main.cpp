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

#include "options.hpp"
#include "arg_parser.hpp"
#include "compiler.hpp"
#include "../common/ast.hpp"
#include "../common/functional_model.hpp"
#include "../frontend/module_parser.hpp"
#include "../frontend/functional_gen.hpp"
#include "../frontend/type_check.hpp"
#include "../frontend/array_reduction.hpp"
#include "../frontend/array_transpose.hpp"
#include "../frontend/ph_model_gen.hpp"
//#include "../polyhedral/modulo_avoidance.hpp"
#include "../polyhedral/scheduling.hpp"
#include "../polyhedral/isl_ast_gen.hpp"
#include "../polyhedral/storage_alloc.hpp"
#include "../cpp/cpp_target.hpp"

#include <stdexcept>

using namespace std;
using namespace stream;
using namespace stream::compiler;

namespace stream {
namespace compiler {

struct verbose_out_options : public option_parser
{
    template <typename T>
    void add_topic(const string & name)
    {
        m_topics.emplace(name, &verbose<T>::enabled());
    }

    void process(arguments & args) override
    {
        string topic;
        while(args.try_parse_argument(topic))
        {
            try { *m_topics.at(topic) = true; }
            catch ( std::out_of_range & e )
            {
                throw arguments::error("Unknown verbose topic: " + topic);
            }
        }
    }

    string description() const override
    {
        ostringstream text;
        text << "  \tAvailable topics:" << endl;
        for (const auto & topic : m_topics)
            text << "  \t- " << topic.first << endl;
        return text.str();
    }

private:
    unordered_map<string,bool*> m_topics;
};

struct int_tuple_parser : public option_parser
{
    string name;
    vector<int> * size;

    int_tuple_parser(const string & name, vector<int> * target): name(name), size(target) {}

    void process(arguments & args) override
    {
        string text;
        args.parse_argument(text, name);
        size_t pos = 0;
        while(pos < text.size())
        {
            auto separator_pos = text.find(',', pos);
            auto num_text = text.substr(pos, separator_pos);
            int num;
            try {
                num = stoi(num_text);
            } catch (std::invalid_argument&) {
                throw arguments::error("Invalid element in " + name + ": " + num_text);
            }

            size->push_back(num);
            if (separator_pos == string::npos)
                break;
            pos = separator_pos + 1;
        }
    }
};

}
}

int main(int argc, char *argv[])
{
    options opt;

    arguments args;

    args.set_default_option({"", "", "<input filename>", ""}, [&opt](arguments& args){
        args.try_parse_argument(opt.input_filename);
    });

    args.add_option({"help", "h", "Print help."}, [](arguments& args){
        args.print_help();
        throw arguments::abortion();
    });

    args.add_option({"import", "i", "<dir>", "Import directory <dir>."},
                    new string_list_option(&opt.import_dirs));

    args.add_option({"import-extension", "ie", "<ext>", "Import files with extension \".<ext>\"."},
                    new string_list_option(&opt.import_extensions));

    args.add_option({"cpp", "", "<name>", "Generate C++ output file named <name>.cpp"},
                    [&opt](arguments& args){
        opt.cpp.enabled = true;
        args.try_parse_argument(opt.cpp.filename);
    });

    args.add_option({"cpp-namespace", "", "<name>", "Generate C++ output in namespace <name>."},
                    new string_option(&opt.cpp.nmspace));

    args.add_option({"sched-whole", "", "", "Schedule whole program at once."},
                    new switch_option(&opt.schedule.cluster, false));
    args.add_option({"sched-tile-size", "", "", "Tile size."},
                    new int_tuple_parser("tile size", &opt.schedule.tile_size));
    args.add_option({"sched-period-direction", "", "", "Direction of periodic tiling."},
                    new int_tuple_parser("period direction", &opt.schedule.periodic_tile_direction));
    args.add_option({"sched-period-offset", "", "", "Offset of period relative to earliest onset."},
                    new int_option(&opt.schedule.period_offset));
    args.add_option({"sched-period-scale", "", "", "Size of period as a multiple of minimal periods."},
                    new int_option(&opt.schedule.period_scale));

    args.add_option({"ast-avoid-branch-in-loop", "", "", "Split loops to avoid branching inside."},
                    new switch_option(&opt.separate_loops));

    args.add_option({"io-unordered", "", "", "Do not necessarily order input and output."},
                    new switch_option(&opt.ordered_io, false));
    args.add_option({"io-atomic", "", "", "Input and output singular elements."},
                    new switch_option(&opt.atomic_io, true));

    args.add_option({"parallel", "", "", "Generate parallelized code, if possible."},
                    new switch_option(&opt.parallel, true));

    args.add_option({"align-data", "", "<bytes>", "Alignment requirement of data."},
                    new int_option(&opt.data_alignment));

    auto verbose_out = new verbose_out_options;
    verbose_out->add_topic<module_parser>("parsing");
    verbose_out->add_topic<ast::output>("ast");
    verbose_out->add_topic<functional::model>("func-model");
    verbose_out->add_topic<functional::generator>("func-model-gen");
    verbose_out->add_topic<functional::type_checker>("type-check");
    verbose_out->add_topic<functional::array_reducer>("array-reduction");
    verbose_out->add_topic<functional::array_transposer>("array-transpose");
    verbose_out->add_topic<functional::polyhedral_gen>("ph-model-gen");
    verbose_out->add_topic<polyhedral::model>("ph-model");
    //verbose_out->add_topic<polyhedral::modulo_avoidance>("mod-avoid");
    verbose_out->add_topic<polyhedral::scheduler>("ph-scheduling");
    verbose_out->add_topic<polyhedral::ast_isl>("ph-ast");
    verbose_out->add_topic<polyhedral::ast_gen>("ph-ast-gen");
    verbose_out->add_topic<polyhedral::storage_allocator>("storage-alloc");
    verbose_out->add_topic<polyhedral::storage_output>("storage");
    verbose_out->add_topic<cpp_gen::renaming>("renaming");

    args.add_option({"verbose", "v", "<topic>", "Enable verbose output for <topic>."}, verbose_out);

    try {
        args.parse(argc-1, argv+1);
    }
    catch (arguments::abortion &)
    {
        return result::ok;
    }
    catch (arguments::error & e)
    {
        cerr << e.msg() << endl;
        return result::command_line_error;
    }

    return compile(opt);
}
