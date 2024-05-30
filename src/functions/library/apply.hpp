#pragma once

#include <functions/evaluatable.hpp>

#include <memory>

namespace magl::functions::library {

class Apply : public IEvaluatable {
 public:
  Apply(std::unique_ptr<IEvaluatable> evaluatable)
      : apply_me_(std::move(evaluatable)) {}

  void Evaluate(ArgsContainer* args, ValueHolder* to) override {
    ValueHolder result;

    // apply_me_ takes no arguments
    apply_me_->Evaluate(args, &result);

    (*reinterpret_cast<std::unique_ptr<IEvaluatable>*>(&result))
        ->Evaluate(args, to);
  }

 private:
  std::unique_ptr<IEvaluatable> apply_me_;
};

}  // namespace magl::functions::library
