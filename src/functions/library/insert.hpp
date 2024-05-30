#pragma once

#include <functions/evaluatable.hpp>
#include <functions/function-factory.hpp>
#include <functions/type.hpp>
#include <functions/utils/type-equivalent.hpp>

#include <value/value.hpp>

namespace magl::functions::library {

template <typename X>
struct InsertImpl : public IEvaluatable {
 public:
  void Evaluate(ArgsContainer* args, ValueHolder* to) override {
    // Assuming that Dict[X] is represented as
    // std::unordered_map<std::string, X>.
    // TODO: add corresonding type trait to eliminate this assumption
    // TODO: If direct value containers are supported, change map<string, Value>
    // to map<string, X>
    using ObjectX = std::unordered_map<std::string, value::Value>;
    new (reinterpret_cast<ObjectX*>(to))
        ObjectX(std::move(*reinterpret_cast<ObjectX*>(&args->at(0))));

    ObjectX* object = reinterpret_cast<ObjectX*>(to);
    object->emplace(std::move(*reinterpret_cast<std::string*>(&args->at(1))),
                    std::move(*reinterpret_cast<X*>(&args->at(2))));
    // TODO: args->at(0)->~DictX();
    // TODO: args->at(1)->~string();
    // TODO: args->at(2)->~X();
  }
};

std::unique_ptr<IEvaluatable> MakeInsertImpl(const functions::Type& x_type);

class Insert : public PolymorphicFunctionFactory {
  const static functions::Type kType;

 public:
  Insert() : PolymorphicFunctionFactory(kType) {}

  std::unique_ptr<IEvaluatable> GetImplementation(
      const Type& specific_type) override;
};

}  // namespace magl::functions::library
