#pragma once

#include <functions/function-factory.hpp>

namespace magl::functions::library {

struct MultiplyIntImpl : IEvaluatable {
  void Evaluate(ArgsContainer* args, ValueHolder* to) override;
};

struct MultiplyFloatImpl : IEvaluatable {
  void Evaluate(ArgsContainer* args, ValueHolder* to) override;
};

class Multiply : public PolymorphicFunctionFactory {
  // X -> X -> X
  const static Type kType;

 public:
  Multiply() : PolymorphicFunctionFactory(kType) {}

  virtual std::unique_ptr<IEvaluatable> GetImplementation(
      const Type& specific_type) override {
    if (specific_type ==
        Type{FunctionType{{IntegerType{}},
                          FunctionType{{IntegerType{}}, IntegerType{}}}}) {
      return std::make_unique<MultiplyIntImpl>();
    }
    // TODO: support Add for float and string
    throw UnsupportedType(
        std::format("f=Multiply, t={}", ToString(specific_type)));
  }

 private:
};

}  // namespace magl::functions::library
