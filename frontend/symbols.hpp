#ifndef STREAM_LANG_SYMBOLIC_INCLUDED
#define STREAM_LANG_SYMBOLIC_INCLUDED

#include "ast.hpp"
#include "types.hpp"

#include <unordered_map>
#include <memory>

namespace stream {
namespace symbolic {

using std::unordered_map;
template <typename T> using sp = std::shared_ptr<T>;

typedef std::unordered_map<string, sp<type::function>> environment;

environment construct_environment( ast::node * program );

} // namespace symbolic
} // namespace stream

#endif //  STREAM_LANG_SYMBOLIC_INCLUDED
