#include <functions/type.hpp>

namespace magl::functions {

namespace {

struct ToStringVisitor : boost::static_visitor<std::string> {
  ToStringVisitor() = default;

  result_type operator()(const functions::TypeVariable tau) const {
    return std::format("Var<{}>", tau.uid);
  }
  result_type operator()(const functions::FunctionType& tau) const {
    return std::format("({}) -> ({})",
                       boost::apply_visitor(*this, tau.argument),
                       boost::apply_visitor(*this, tau.body));
  }
  result_type operator()(const functions::IntegerType&) const { return "INT"; }
  result_type operator()(const functions::DoubleType&) const { return "FLOAT"; }
  result_type operator()(const functions::BoolType&) const { return "BOOL"; }
  result_type operator()(const functions::StringType&) const { return "STR"; }
  result_type operator()(const functions::NullType&) const { return "NULL"; }
  result_type operator()(const functions::AnyType&) const { return "ANY"; }
  result_type operator()(const functions::ListType& tau) const {
    return std::format("List[{}]", boost::apply_visitor(*this, tau.value_type));
  }
  result_type operator()(const functions::DictType& tau) const {
    return std::format("Dict[{}]", boost::apply_visitor(*this, tau.value_type));
  }
  result_type operator()(const functions::SchemaType& tau) const {
    std::string values;
    for (const auto& [key, value] : tau.values) {
      values +=
          std::format("{}: ({})  ", key, boost::apply_visitor(*this, value));
    }
    return std::format("Schema[{}]", values);
  }
};

}  // namespace

std::string ToString(const Type& t) {
  return boost::apply_visitor(ToStringVisitor{}, t);
}

}  // namespace magl::functions
