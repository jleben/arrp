#include "platform.hpp"

namespace arrp {

platform * get_platform()
{
    static platform p;
    return &p;
}

}
