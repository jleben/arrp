#pragma once

#include <string>

namespace arrp {

using std::string;

struct platform
{
    virtual string executable_path() { return string(); }
    virtual string builtin_import_path() { return string(); }
};

platform * get_platform();

}
