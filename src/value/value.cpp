#include <value/value.hpp>

namespace magl::value {

bool IsPrimitive(const Value& value) {
  return boost::get<LambdaValue>(&value) == nullptr;
}

}  // namespace magl::value
