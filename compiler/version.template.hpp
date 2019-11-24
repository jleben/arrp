#include <string>

namespace arrp {

class info
{
public:
    static std::string version() { return "@ARRP_VERSION@"; }
    static std::string commit() { return "@ARRP_COMMIT@"; }
};

}
