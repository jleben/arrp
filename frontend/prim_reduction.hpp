#ifndef STREAM_LANG_PRIMITIVE_REDUCER_INCLUDED
#define STREAM_LANG_PRIMITIVE_REDUCER_INCLUDED

#include "../common/primitives.hpp"
#include "../common/functional_model.hpp"

namespace stream {
namespace functional {

expr_ptr reduce_primitive(std::shared_ptr<primitive>);

}
}

#endif // STREAM_LANG_PRIMITIVE_REDUCER_INCLUDED
