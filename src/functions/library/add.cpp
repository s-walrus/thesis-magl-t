#include <functions/library/add.hpp>

#include <value/value.hpp>

namespace magl::functions::library {

namespace {

struct AddAnyVisitor : boost::static_visitor<value::Value> {
  AddAnyVisitor() = default;

  result_type operator()(value::IntegerValue lhs, value::Value rhs) const {
    if (boost::get<value::IntegerValue>(&rhs)) {
      return value::IntegerValue{lhs + boost::get<value::IntegerValue>(rhs)};
    }
    if (boost::get<value::FloatValue>(&rhs)) {
      return value::FloatValue{lhs + boost::get<value::FloatValue>(rhs)};
    }
    throw std::invalid_argument(
        "Integer value can only be added to integer or float value.");
  }
  result_type operator()(value::FloatValue /*lhs*/,
                         value::Value /*rhs*/) const {
    throw std::logic_error("Not implemented.");
  }
  result_type operator()(value::BoolValue /*lhs*/, value::Value /*rhs*/) const {
    throw std::logic_error("Not implemented.");
  }
  result_type operator()(value::StringValue /*lhs*/,
                         value::Value /*rhs*/) const {
    throw std::logic_error("Not implemented.");
  }
  result_type operator()(value::NullValue /*lhs*/, value::Value /*rhs*/) const {
    throw std::logic_error("Not implemented.");
  }
  result_type operator()(value::LambdaValue /*lhs*/,
                         value::Value /*rhs*/) const {
    throw std::logic_error("Not implemented.");
  }
  result_type operator()(value::ObjectValue /*lhs*/,
                         value::Value /*rhs*/) const {
    throw std::logic_error("Not implemented.");
  }
  result_type operator()(value::ArrayValue /*lhs*/,
                         value::Value /*rhs*/) const {
    throw std::logic_error("Not implemented.");
  }
};

}  // namespace

const Type Add::kType = FunctionType{
    TypeVariable{'X'}, FunctionType{TypeVariable{'X'}, TypeVariable{'X'}}};

void AddIntImpl::Evaluate(ArgsContainer* args, ValueHolder* to) {
  *reinterpret_cast<value::IntegerValue*>(to) =
      std::move(*reinterpret_cast<value::IntegerValue*>(&args->at(0))) +
      std::move(*reinterpret_cast<value::IntegerValue*>(&args->at(1)));
}

void AddFloatImpl::Evaluate(ArgsContainer* args, ValueHolder* to) {
  *reinterpret_cast<value::FloatValue*>(to) =
      std::move(*reinterpret_cast<value::FloatValue*>(&args->at(0))) +
      std::move(*reinterpret_cast<value::FloatValue*>(&args->at(1)));
}

void AddStringImpl::Evaluate(ArgsContainer* args, ValueHolder* to) {
  new (reinterpret_cast<value::Value*>(to)) value::StringValue(
      std::move(*reinterpret_cast<value::StringValue*>(&args->at(0))) +
      std::move(*reinterpret_cast<value::StringValue*>(&args->at(1))));
}

void AddAnyImpl::Evaluate(ArgsContainer* args, ValueHolder* to) {
  new (reinterpret_cast<value::Value*>(to)) value::Value(boost::apply_visitor(
      AddAnyVisitor{},
      std::move(*reinterpret_cast<value::Value*>(&args->at(0))),
      std::move(*reinterpret_cast<value::Value*>(&args->at(1)))));
}

}  // namespace magl::functions::library
