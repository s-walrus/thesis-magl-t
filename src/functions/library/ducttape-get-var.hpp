#pragma once

#include <type_traits>

#include <functions/function-factory.hpp>
#include <value/value.hpp>

namespace magl::functions::library {

template <typename V>
// Trivially copiable
requires std::is_same_v<V, value::IntegerValue> ||
         std::is_same_v<V, value::BoolValue>
struct GetVarImpl : IEvaluatable {
  void Evaluate(ArgsContainer* args, ValueHolder* to) override {
    // Copying
    *reinterpret_cast<V*>(to) = **reinterpret_cast<V**>(&args->at(0));
  }
};

class GetVar : public PolymorphicFunctionFactory {
  const static Type kType;

 public:
  GetVar() : PolymorphicFunctionFactory(kType) {}

  virtual std::unique_ptr<IEvaluatable> GetImplementation(
      const Type& specific_type) override {
    if (specific_type == Type{FunctionType{IntegerType{}, IntegerType{}}}) {
      return std::make_unique<GetVarImpl<value::IntegerValue>>();
    } else if (specific_type == Type{FunctionType{BoolType{}, BoolType{}}}) {
      return std::make_unique<GetVarImpl<value::BoolValue>>();
    }
    throw UnsupportedType("GetVar is only implemented for integer and string");
  }

 private:
};

}  // namespace magl::functions::library
