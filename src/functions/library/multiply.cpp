#include <functions/library/multiply.hpp>

#include <value/value.hpp>

namespace magl::functions::library {

const Type Multiply::kType = FunctionType{
    TypeVariable{'X'}, FunctionType{TypeVariable{'X'}, TypeVariable{'X'}}};

void MultiplyIntImpl::Evaluate(ArgsContainer* args, ValueHolder* to) {
  *reinterpret_cast<value::IntegerValue*>(to) =
      std::move(*reinterpret_cast<value::IntegerValue*>(&args->at(0))) *
      std::move(*reinterpret_cast<value::IntegerValue*>(&args->at(1)));
}

void MultiplyFloatImpl::Evaluate(ArgsContainer* args, ValueHolder* to) {
  *reinterpret_cast<value::FloatValue*>(to) =
      std::move(*reinterpret_cast<value::FloatValue*>(&args->at(0))) *
      std::move(*reinterpret_cast<value::FloatValue*>(&args->at(1)));
}

}  // namespace magl::functions::library
