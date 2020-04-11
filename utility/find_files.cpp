#include "find_files.h"
#include "../common/error.hpp"

#include <cstdlib>

using namespace std;

namespace arrp {

std::string find_cpp_compiler()
{
    string cpp_compiler;

    auto * CXX = getenv("CXX");

    if (CXX)
        cpp_compiler = CXX;

    if (cpp_compiler.empty())
    {
        if (system("c++ --version 2>&1 > /dev/null") == 0)
            cpp_compiler = "c++";
    }

    if (cpp_compiler.empty())
    {
        if (system("g++ --version 2>&1 > /dev/null") == 0)
            cpp_compiler = "g++";
    }

    if (cpp_compiler.empty())
    {
        throw stream::error("Failed to find a C++ compiler.");
    }

    return cpp_compiler;
}

std::string find_arrp_cpp_include_dir()
{
    auto * CPATH = getenv("CPATH");

    if (CPATH)
    {
        string cpath(CPATH);
        ifstream file(cpath + "/arrp/arrp.hpp");
        if (file.is_open())
            return cpath;
    }


}


}
