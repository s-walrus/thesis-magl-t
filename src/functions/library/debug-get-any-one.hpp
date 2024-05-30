#pragma once

#include <functions/evaluatable.hpp>
#include <functions/function-factory.hpp>
#include <value/value.hpp>

namespace magl::functions::library {

class GetAnyOneImpl : public IEvaluatable {
 public:
  GetAnyOneImpl() {}

  virtual void Evaluate(ArgsContainer*, ValueHolder* to) override {
    new (reinterpret_cast<value::Value*>(to))
        value::Value(value::IntegerValue{1});
  }

 private:
};

class GetAnyOne : public NonPolymorphicFunctionFactory {
 public:
  GetAnyOne() : NonPolymorphicFunctionFactory(AnyType{}) {}

  std::unique_ptr<IEvaluatable> GetImplementation() override {
    return std::make_unique<GetAnyOneImpl>();
  }

 private:
  std::unique_ptr<IEvaluatable> evaluatable_;
};

}  // namespace magl::functions::library
