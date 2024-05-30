#pragma once

#include <functions/function-factory.hpp>

namespace magl::functions::library {

struct AddIntImpl : IEvaluatable {
  void Evaluate(ArgsContainer* args, ValueHolder* to) override;
};

struct AddFloatImpl : IEvaluatable {
  void Evaluate(ArgsContainer* args, ValueHolder* to) override;
};

struct AddStringImpl : IEvaluatable {
  void Evaluate(ArgsContainer* args, ValueHolder* to) override;
};

struct AddAnyImpl : IEvaluatable {
  void Evaluate(ArgsContainer* args, ValueHolder* to) override;
};

class Add : public PolymorphicFunctionFactory {
  const static Type kType;

 public:
  Add() : PolymorphicFunctionFactory(kType) {}

  virtual std::unique_ptr<IEvaluatable> GetImplementation(
      const Type& specific_type) override {
    if (specific_type ==
        Type{FunctionType{{IntegerType{}},
                          FunctionType{{IntegerType{}}, IntegerType{}}}}) {
      return std::make_unique<AddIntImpl>();
    } else if (specific_type ==
               Type{FunctionType{{AnyType{}},
                                 FunctionType{{AnyType{}}, AnyType{}}}}) {
      return std::make_unique<AddAnyImpl>();
    }
    // TODO: support Add for float and string
    throw UnsupportedType(std::format("f=Add, t={}", ToString(specific_type)));
  }

 private:
};

}  // namespace magl::functions::library
