#include "platform.hpp"
#include "../common/error.hpp"
#include <mach-o/dyld.h>

#include <vector>

using namespace std;

namespace arrp {

static
string path_dir(const string & path)
{
    string::size_type last_sep = path.rfind('/');
    if (last_sep == string::npos)
    {
        return string();
    }

    if (last_sep == 0)
        ++last_sep;

    return path.substr(0, last_sep);
}

class macos_platform : public platform
{
    string executable_path() override;
    string builtin_import_path() override;
};

platform * get_platform()
{
    static macos_platform p;
    return &p;
}

string macos_platform::executable_path()
{
    uint32_t size = 1024;
    vector<char> buf(size);

    int error = _NSGetExecutablePath(buf.data(), &size);
    if (error)
    {
        // 'size' was updated to required size.
        buf.resize(size);
        error = _NSGetExecutablePath(buf.data(), &size);
        if (error)
        {
            throw stream::error("_NSGetExecutablePath failed twice.");
        }
    }

    string path(buf.data(), size);

    string dir_path = path_dir(path);
    if (dir_path.empty())
    {
        throw stream::error(string("Could not find parent directory in path: ") + path);
    }

    return dir_path;
}

string macos_platform::builtin_import_path()
{
    string exe_path = executable_path();
    return exe_path + "/../lib/arrp/library";
}

}
