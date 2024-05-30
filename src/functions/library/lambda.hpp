#pragma once

#include <executer/evaluate.hpp>
#include <executer/expression.hpp>
#include <functions/evaluatable.hpp>
#include <functions/utils/value-type.hpp>
#include <parser/terms/terms.hpp>
#include <value/value.hpp>

namespace magl::functions::library {

class HoldVariable : public IEvaluatable {
 public:
  HoldVariable() : arg_(std::make_unique<ValueHolder>()) {}

  const ValueHolder* GetVariableLocation() const { return arg_.get(); }

  void Evaluate(ArgsContainer* args, ValueHolder*) override {
    const ValueHolder* arg_p = &args->at(0);
    assert(arg_);
    *arg_.get() = *arg_p;
  }

 private:
  // FIXME: Value from arg_ is not destroyed ever which leads to memory leak
  std::unique_ptr<ValueHolder> arg_;
};

class Lambda : public IEvaluatable {
 public:
  Lambda(std::unique_ptr<ValueHolder> arg_holder, executer::EvaluationTree body)
      : arg_var_holder_(std::move(arg_holder)), body_(std::move(body)) {}

  static std::unique_ptr<ValueHolder> MakeVarHolder() {
    return std::make_unique<ValueHolder>();
  }

  const ValueHolder* GetVariableLocation() const {
    return arg_var_holder_.get();
  }

  void Evaluate(ArgsContainer* args, ValueHolder* to) override {
    // Put argument to the variable location
    *arg_var_holder_.get() = args->at(0);

    // Evaluate body
    *to = executer::EvaluateTerm(body_);
  }

 private:
  std::unique_ptr<ValueHolder> arg_var_holder_;
  executer::EvaluationTree body_;
};

}  // namespace magl::functions::library
