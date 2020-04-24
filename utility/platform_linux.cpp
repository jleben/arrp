#include "platform.hpp"
#include "../common/error.hpp"

#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;

namespace arrp {

class linux_platform : public platform
{
    string executable_path() override;
    string builtin_import_path() override;
};

platform * get_platform()
{
    static linux_platform p;
    return &p;
}

string linux_platform::executable_path()
{
    char * path_c = realpath("/proc/self/exe", nullptr);

    if (!path_c)
    {
        int e = errno;
        throw stream::error(string("Could not resolve /proc/self/exe: ") + strerror(e));
    }

    string path(path_c);
    free(path_c);

    string::size_type last_sep = path.rfind('/');
    if (last_sep == string::npos)
    {
        throw stream::error(string("Could not find parent directory in path: ") + path);
    }

    if (last_sep == 0)
        ++last_sep;

    return path.substr(0, last_sep);
}

string linux_platform::builtin_import_path()
{
    string exe_path = executable_path();
    return exe_path + "/../lib/arrp/library";
}

}
