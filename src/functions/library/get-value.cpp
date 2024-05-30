#include <functions/library/get-value.hpp>

namespace magl::functions::library {

namespace {

struct MakeGetValueVisitor
    : boost::static_visitor<std::unique_ptr<IEvaluatable>> {
  MakeGetValueVisitor() = default;

  result_type operator()(value::IntegerValue v) const {
    return std::make_unique<GetValue<value::IntegerValue>>(std::move(v));
  }
  result_type operator()(value::FloatValue v) const {
    return std::make_unique<GetValue<value::FloatValue>>(std::move(v));
  }
  result_type operator()(value::BoolValue v) const {
    return std::make_unique<GetValue<value::BoolValue>>(std::move(v));
  }
  result_type operator()(value::StringValue v) const {
    return std::make_unique<GetValue<value::StringValue>>(std::move(v));
  }
  result_type operator()(value::NullValue v) const {
    return std::make_unique<GetValue<value::NullValue>>(std::move(v));
  }
  result_type operator()(value::LambdaValue v) const {
    return std::make_unique<GetValue<value::LambdaValue>>(std::move(v));
  }
  result_type operator()(value::ObjectValue v) const {
    return std::make_unique<GetValue<value::ObjectValue>>(std::move(v));
  }
  result_type operator()(value::ArrayValue v) const {
    return std::make_unique<GetValue<value::ArrayValue>>(std::move(v));
  }
};

}  // namespace

std::unique_ptr<IEvaluatable> MakeGetValue(value::Value to_return) {
  return boost::apply_visitor(MakeGetValueVisitor{}, to_return);
}

}  // namespace magl::functions::library
