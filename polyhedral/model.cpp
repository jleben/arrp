#include "model.hpp"

#include <iomanip>

using namespace std;

namespace stream {
namespace polyhedral {

ostream & operator<<(ostream &s, const mapping & m)
{
    for (int row = 0; row < m.output_dimension(); ++row)
    {
        for (int col = 0; col < m.input_dimension(); ++col)
        {
            s << std::setw(4) << m.coefficients(row,col);
        }
        s << " | " << m.constants[row];
        s << endl;
    }
    return s;
}

}
}
