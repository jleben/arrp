#pragma once

#include "../../extra/json/json.hpp"

#include <string>

namespace arrp {
namespace puredata_io {

// For the purpose of verbose output:
struct log {};

struct options
{
    std::string base_file_name;
    std::string pd_object_name;
};

void generate(const options &, const nlohmann::json & report);

}
}

