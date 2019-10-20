#pragma once

#include "../common/error.hpp"

#include <string>

namespace arrp {
namespace subprocess {

using std::string;

struct error : public stream::error
{
    error() {}
    error(const string &what): stream::error(what) {}
};

void run(const string & command);

}
}
