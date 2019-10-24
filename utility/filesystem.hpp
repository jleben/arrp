#pragma once

#include "../common/error.hpp"

#include <string>

namespace arrp {
namespace filesystem {

class temporary_dir
{
public:
    temporary_dir();
    ~temporary_dir();
    // Lazily creates name if one hasn't been created yet.
    // Throws stream::error on failure to create.
    const std::string & name();

private:
    std::string d_name;
};

}
}
