#include <functions/utils/value-type.hpp>

namespace magl::functions::utils {

namespace {

struct GetTypeVisitor : boost::static_visitor<Type> {
  GetTypeVisitor() = default;

  result_type operator()(const value::IntegerValue&) const {
    return IntegerType{};
  }
  result_type operator()(const value::FloatValue&) const {
    return DoubleType{};
  }
  result_type operator()(const value::BoolValue&) const { return BoolType{}; }
  result_type operator()(const value::StringValue&) const {
    return StringType{};
  }
  result_type operator()(const value::NullValue&) const { return NullType{}; }
  result_type operator()(const value::LambdaValue&) const {
    return AnyType{};  // Not implemented
  }
  result_type operator()(const value::ObjectValue&) const {
    // TODO: Consider returning either a Dict or a Schema
    return DictType{AnyType{}};  // Not implemented
  }
  result_type operator()(const value::ArrayValue&) const {
    return ListType{AnyType{}};  // Not implemented
  }
};

}  // namespace

Type GetType(const value::Value& value) {
  return boost::apply_visitor(GetTypeVisitor{}, value);
}

}  // namespace magl::functions::utils
