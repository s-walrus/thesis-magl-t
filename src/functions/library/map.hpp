#pragma once

#include <functions/function-factory.hpp>
#include <value/value.hpp>

namespace magl::functions::library {

template <typename X, typename Y>
struct MapImpl : IEvaluatable {
  void Evaluate(ArgsContainer* args, ValueHolder* to) override {
    IEvaluatable* f = *reinterpret_cast<value::LambdaValue*>(&args->at(0));
    value::ArrayValue items =
        std::move(*reinterpret_cast<value::ArrayValue*>(&args->at(1)));
    value::ArrayValue result;
    result.reserve(items.size());

    for (value::Value& v : items) {
      new (&args->at(0)) X(std::move(boost::get<X>(v)));
      f->Evaluate(args, to);
      result.push_back(std::move(*reinterpret_cast<Y*>(to)));
    }

    new (to) value::ArrayValue(std::move(result));
  }
};

class Map : public PolymorphicFunctionFactory {
  // (X -> Y) -> List[X] -> List[Y]
  const static Type kType;

 public:
  Map() : PolymorphicFunctionFactory(kType) {}

  virtual std::unique_ptr<IEvaluatable> GetImplementation(
      const Type& specific_type) override {
    if (!boost::get<FunctionType>(&specific_type)) {
      throw UnsupportedType(
          std::format("{} !~ {}", ToString(kType), ToString(specific_type)));
    }
    if (!boost::get<FunctionType>(
            &boost::get<FunctionType>(specific_type).argument)) {
      throw UnsupportedType(
          std::format("{} !~ {}", ToString(kType), ToString(specific_type)));
    }
    const Type x = boost::get<FunctionType>(
                       boost::get<FunctionType>(specific_type).argument)
                       .argument;
    const Type y = boost::get<FunctionType>(
                       boost::get<FunctionType>(specific_type).argument)
                       .body;

    if (!boost::get<FunctionType>(
            &boost::get<FunctionType>(specific_type).body)) {
      throw UnsupportedType(
          std::format("{} !~ {}", ToString(kType), ToString(specific_type)));
    }
    if (boost::get<FunctionType>(boost::get<FunctionType>(specific_type).body)
            .argument != Type{ListType{x}}) {
      throw UnsupportedType(
          std::format("{} !~ {}", ToString(kType), ToString(specific_type)));
    }
    if (boost::get<FunctionType>(boost::get<FunctionType>(specific_type).body)
            .body != Type{ListType{y}}) {
      throw UnsupportedType(
          std::format("{} !~ {}", ToString(kType), ToString(specific_type)));
    }

    if (boost::get<DictType>(&x) && boost::get<DictType>(&y)) {
      return std::make_unique<
          MapImpl<value::ObjectValue, value::ObjectValue>>();
    }

    if (boost::get<IntegerType>(&x) && boost::get<IntegerType>(&y)) {
      return std::make_unique<
          MapImpl<value::IntegerValue, value::IntegerValue>>();
    }

    throw UnsupportedType(
        std::format("Not implemented: {}", ToString(specific_type)));
  }

 private:
};

}  // namespace magl::functions::library
