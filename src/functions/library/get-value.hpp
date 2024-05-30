#pragma once

#include <functions/evaluatable.hpp>
#include <value/value.hpp>

namespace magl::functions::library {

template <typename OutputT>
class GetValue : public IEvaluatable {
 public:
  GetValue(value::Value to_return)
      : return_me_(std::move(boost::get<OutputT>(to_return))) {}

  virtual void Evaluate(ArgsContainer*, ValueHolder* to) override {
    new (reinterpret_cast<OutputT*>(to)) OutputT(return_me_);
  }

 private:
  const OutputT return_me_;
};

std::unique_ptr<IEvaluatable> MakeGetValue(value::Value to_return);

/// Owns an executable and returns it as a lambda value
class GetLambdaValue : public IEvaluatable {
 public:
  GetLambdaValue(std::unique_ptr<IEvaluatable> evaluatable)
      : evaluatable_(std::move(evaluatable)) {}

  virtual void Evaluate(ArgsContainer*, ValueHolder* to) override {
    *reinterpret_cast<IEvaluatable**>(to) = evaluatable_.get();
  }

 private:
  std::unique_ptr<IEvaluatable> evaluatable_;
};

}  // namespace magl::functions::library
