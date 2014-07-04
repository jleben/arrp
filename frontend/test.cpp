#include "parser.h"
#include "ast_printer.hpp"
#include "semantic.hpp"
#include <fstream>
#include <iostream>

using namespace std;
using namespace stream;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cerr << "Missing argument: input filename." << endl;
    return 1;
  }

  string input_filename( argv[1] );

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

  stream::semantic::environment_item *main = env["main"];
  if (!main)
  {
      cerr << "WARNING: no 'main' function." << endl;
      return 0;
  }

  vector<std::shared_ptr<semantic::type>> args;
  args.emplace_back( new semantic::stream({10,20,30}) );
  //args.emplace_back( new semantic::stream({10,20,30}) );

  try {
      sp<semantic::type> value = main->evaluate(env, args);
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
