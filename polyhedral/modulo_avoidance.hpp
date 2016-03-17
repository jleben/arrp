#ifndef STREAM_LANG_POLYHEDRAL_MODULO_AVOIDANCE_INCLUDED
#define STREAM_LANG_POLYHEDRAL_MODULO_AVOIDANCE_INCLUDED

#include "../common/ph_model.hpp"

namespace stream {
namespace polyhedral {


void avoid_modulo(schedule &, model &, bool split_statements);

}
}

#endif // STREAM_LANG_POLYHEDRAL_MODULO_AVOIDANCE_INCLUDED
