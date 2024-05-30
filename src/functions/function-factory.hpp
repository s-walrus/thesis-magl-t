#pragma once

#include <memory>
#include <string>

#include <functions/evaluatable.hpp>
#include <functions/type.hpp>
#include <functions/utils/type-equivalent.hpp>

namespace magl::functions {

/// Thrown from GetImplementation if there is no implementation for requested
/// type
struct UnsupportedType : std::invalid_argument {
  using std::invalid_argument::invalid_argument;
};

// NB: Allowed only polymorphic variables with one variable type. Otherwise
// inference may be NP-complete
class PolymorphicFunctionFactory {
 public:
  PolymorphicFunctionFactory(Type polymorphic_type)
      : type_(std::move(polymorphic_type)) {}

  const Type& GetPolymorphicType() { return type_; }

  virtual std::unique_ptr<IEvaluatable> GetImplementation(
      const Type& specific_type) = 0;

  virtual ~PolymorphicFunctionFactory() = default;

 private:
  Type type_;
};

class NonPolymorphicFunctionFactory : public PolymorphicFunctionFactory {
 public:
  NonPolymorphicFunctionFactory(Type type)
      : PolymorphicFunctionFactory(std::move(type)) {}

  virtual std::unique_ptr<IEvaluatable> GetImplementation(
      const Type& specific_type) override {
    if (!utils::IsInstanceOf(specific_type, GetPolymorphicType())) {
      throw UnsupportedType(
          "Requested type does not match actual type of the function.");
    }
    return GetImplementation();
  }

  virtual std::unique_ptr<IEvaluatable> GetImplementation() = 0;
};

}  // namespace magl::functions
