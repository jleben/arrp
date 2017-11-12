#include "../utility/cpp-gen.hpp"

#include <unordered_set>
#include <string>

namespace stream {
namespace cpp_gen {

bool collect_names(const expression_ptr &, std::unordered_set<std::string> & names);

}
}
