#include "subprocess.hpp"

#include <cstdlib>

namespace arrp {
namespace subprocess {

void run(const string & command)
{
    int status = system(command.c_str());
    if (status != 0)
    {
        throw error("Failed to execute subprocess: " + command);
    }
}

}
}
