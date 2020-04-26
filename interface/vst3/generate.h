#pragma once

#include "../../extra/json/json.hpp"

namespace arrp {
namespace vst3 {

struct options
{
    std::string base_file_name;
};

void generate(const options &, const nlohmann::json & report);

}
}
