#include "parser.h"
#include "ast_printer.hpp"
#include "semantic.hpp"
#include <fstream>
#include <iostream>

using namespace std;
using namespace stream;

sp<semantic::type> parse_stream_arg(const string & arg)
{
    if (arg.size() < 3)
        throw std::runtime_error("Misformatted stream argument.");

    if (arg.front() != '[' || arg.back() != ']')
        throw std::runtime_error("Misformatted stream argument.");

    string size_list = arg.substr(1, arg.size()-2);

    vector<int> sizes;
    istringstream size_stream(size_list);
    string elem;
    while(std::getline(size_stream, elem, ','))
    {
        try {
            size_t pos = 0;
            int size = stoi(elem, &pos);
            if (pos == elem.size())
            {
                sizes.push_back(size);
                continue;
            }
        } catch (...) {}

        throw std::runtime_error("Misformatted stream argument.");
    }

    return sp<semantic::type>( new semantic::stream(sizes) );
}

sp<semantic::type> parse_scalar_arg(const string & arg)
{
    try {
        size_t pos = 0;
        int i = stoi(arg, &pos);
        if (pos == arg.size())
            return sp<semantic::type>( new semantic::integer_num(i) );
    }
    catch (...) {}

    try {
        size_t pos = 0;
        double d = stod(arg, &pos);
        if (pos == arg.size())
            return sp<semantic::type>( new semantic::real_num(d) );
    }
    catch (...) {}

    throw std::runtime_error("Misformatted scalar argument.");
}

sp<semantic::type> parse_range_arg(const string & arg)
{
    throw std::runtime_error("Unsupported range argument.");
}

sp<semantic::type> parse_arg(const string & arg)
{
    assert(!arg.empty());

    switch(arg[0])
    {
    case '[':
        return parse_stream_arg(arg);
    //case '[':
        //return parse_range_arg(arg);
    default:
        return parse_scalar_arg(arg);
    }
}

int main(int argc, char *argv[])
{
    string input_filename;
    string function_name;

    if (argc < 2)
    {
        cerr << "Missing argument: input filename." << endl;
        return 1;
    }
    input_filename = argv[1];

    if (argc > 2)
    {
        function_name = argv[2];
    }
    else
    {
        function_name = "main";
        cerr << "WARNING: No starting function name given." << endl
             << "WARNING: Assuming default: " << function_name << endl;
    }

    vector<sp<semantic::type>> function_args;
    for (int i = 3; i < argc; ++i)
    {
        try {
            function_args.emplace_back( parse_arg(argv[i]) );
        } catch (exception & e)
        {
            cerr << "ERROR while parsing arguments: " << e.what() << endl;
            return 0;
        }
    }

  ifstream input_file(input_filename);

  stream::Parser parser(input_file);

  int success = parser.parse();

  if (success != 0)
      return success;

  cout << endl;

  cout << "== Abstract Syntax Tree ==" << endl;
  stream::ast::printer printer;
  printer.print( parser.ast().get() );
  cout << endl;

  cout << endl;

  cout << "== Semantic Analysis ==" << endl;
  stream::semantic::environment env = stream::semantic::top_environment( parser.ast().get() );

  stream::semantic::symbol *main = env[function_name];
  if (!main)
  {
      cerr << "WARNING: no function named '" << function_name << "' available." << endl;
      return 0;
  }

  //vector<std::shared_ptr<semantic::type>> args;
  //args.emplace_back( new semantic::stream({10,20,30}) );
  //args.emplace_back( new semantic::stream({10,20,30}) );

  cout << "Evaluating: " << function_name;
  cout << "(";
  if (function_args.size())
      cout << *function_args.front();
  for ( int i = 1; i < function_args.size(); ++i )
  {
      cout << ", ";
      cout << *function_args[i];
  }
  cout << ")" << endl;

  try {
      sp<semantic::type> value = main->evaluate(env, function_args);
      if (value)
          cout << "Result = " << *value << endl;
      else
          cout << "No result." << endl;
  }
  catch (semantic::semantic_error & e)
  {
      e.report();
  }
}
