#include "report.hpp"

namespace arrp {

json & report()
{
    static json data;
    return data;
}

}
