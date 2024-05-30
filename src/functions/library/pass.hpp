#pragma once

#include <functions/evaluatable.hpp>
#include <functions/type.hpp>

namespace magl::functions::library {

/// Returns last argument
class Pass : public IEvaluatable {
 public:
  Pass(std::vector<Type> arg_types) : args_size_(arg_types.size()) {}

  void Evaluate(ArgsContainer* args, ValueHolder* to) override {
    *to = args->at(args_size_ - 1);
  }

 private:
  const size_t args_size_;
};

}  // namespace magl::functions::library
