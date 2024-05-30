#pragma once

#include <functions/evaluatable.hpp>
#include <functions/function-factory.hpp>
#include <functions/type.hpp>
#include <value/value.hpp>

namespace magl::functions::library {

/// List[X] -> X -> List[X]
template <typename X>
struct AppendImpl : IEvaluatable {
  virtual void Evaluate(ArgsContainer* args, ValueHolder* to) override {
    // Assuming that List[X] is represented as
    // std::vector<X>.
    // TODO: add corresonding type trait to eliminate this assumption
    // TODO: If direct value containers are supported, change vector<Value>
    // to vector<X>
    using ListX = std::vector<value::Value>;
    new (reinterpret_cast<ListX*>(to))
        ListX(std::move(*reinterpret_cast<ListX*>(&args->at(0))));

    ListX* list = reinterpret_cast<ListX*>(to);
    list->emplace_back(std::move(*reinterpret_cast<X*>(&args->at(1))));
    // TODO: args->at(0)->~DictX();
    // TODO: args->at(1)->~string();
    // TODO: args->at(2)->~X();
  }
};

std::unique_ptr<IEvaluatable> MakeAppendImpl(const functions::Type& x_type);

class Append : public PolymorphicFunctionFactory {
  const static functions::Type kType;

 public:
  Append() : PolymorphicFunctionFactory(kType) {}

  std::unique_ptr<IEvaluatable> GetImplementation(
      const Type& /*specific_type*/) override;
};

}  // namespace magl::functions::library
