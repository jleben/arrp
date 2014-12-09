#ifndef STREAM_LANG_CPP_INTERFACE_GENERATOR_INCLUDED
#define STREAM_LANG_CPP_INTERFACE_GENERATOR_INCLUDED

#include "../polyhedral/dataflow_model.hpp"
#include "../polyhedral/model.hpp"
#include "../frontend/types.hpp"
#include "../utility/cpp-gen.hpp"

#include <vector>
#include <string>

namespace stream {
namespace cpp_interface {

using std::vector;
using std::string;

cpp_gen::program * create
( const string & name,
  const vector<semantic::type_ptr> & args,
  const vector<polyhedral::statement*> &,
  const dataflow::model & );

}
}

#endif // STREAM_LANG_CPP_INTERFACE_GENERATOR_INCLUDED
