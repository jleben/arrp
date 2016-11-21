
#include <isl-cpp/set.hpp>
#include <vector>

namespace arrp
{
using std::vector;
using ivector = std::vector<int>;

void find_rays(const isl::basic_set &, vector<ivector> & rays);

}
