#include "parser.h"
#include "ast_printer.hpp"
#include "symbols.hpp"
#include <fstream>
#include <iostream>

using namespace std;

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

  cout << "== Constructing evironment ==" << endl;
  stream::symbolic::construct_environment( parser.ast().get() );
}
