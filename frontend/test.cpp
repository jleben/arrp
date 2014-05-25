#include "parser.h"
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

  return parser.parse();
}
