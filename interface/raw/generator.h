#pragma once

#include "../../extra/json/json.hpp"

#include <string>

namespace arrp {
namespace generic_io {

struct options
{
    std::string output_file;
    std::string mode;
};

void generate(const options &, const nlohmann::json & report);

}
}
