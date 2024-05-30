#pragma once

#include <type_traits>

#include <functions/function-factory.hpp>
#include <value/value.hpp>

namespace magl::functions::library {

template <typename V>
struct VarMapAtImpl : IEvaluatable {
  void Evaluate(ArgsContainer* args, ValueHolder* to) override {
    // Copying
    const std::string& key =
        *reinterpret_cast<value::StringValue*>(&args->at(1));
    new (reinterpret_cast<V*>(to)) V(boost::get<V>(
        (*reinterpret_cast<value::ObjectValue**>(&args->at(0)))->at(key)));
  }
};

class VarMapAt : public PolymorphicFunctionFactory {
  const static Type kType;

 public:
  VarMapAt() : PolymorphicFunctionFactory(kType) {}

  virtual std::unique_ptr<IEvaluatable> GetImplementation(
      const Type& specific_type) override {
    if (specific_type ==
        Type{FunctionType{DictType{IntegerType{}},
                          FunctionType{StringType{}, IntegerType{}}}}) {
      return std::make_unique<VarMapAtImpl<value::IntegerValue>>();
    } else if (specific_type ==
               Type{FunctionType{DictType{BoolType{}},
                                 FunctionType{StringType{}, BoolType{}}}}) {
      return std::make_unique<VarMapAtImpl<value::BoolValue>>();
    } else if (specific_type ==
               Type{FunctionType{DictType{StringType{}},
                                 FunctionType{StringType{}, StringType{}}}}) {
      return std::make_unique<VarMapAtImpl<value::StringValue>>();
    }
    // TODO: support other types
    throw UnsupportedType(
        std::format("f=VarMapAt, t={}", ToString(specific_type)));
  }

 private:
};

}  // namespace magl::functions::library
