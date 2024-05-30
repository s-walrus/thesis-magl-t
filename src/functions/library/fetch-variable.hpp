#pragma once

#include <functions/evaluatable.hpp>

namespace magl::functions::library {

class FetchVariable : public IEvaluatable {
 public:
  FetchVariable(const ValueHolder* location) : variable_location_(location) {}

  void Evaluate(ArgsContainer*, ValueHolder* to) override {
    // TODO: Consider using type-safe container instead of plain byte std::array
    // for ValueHolder
    // FIXME: Returning a pointer to a variable location is a very poor
    // duct-tape solution. It must be fixed in a somewhat stable version of MAGL
    *reinterpret_cast<const ValueHolder**>(to) = variable_location_;
  }

 private:
  const ValueHolder* variable_location_;
};

}  // namespace magl::functions::library
