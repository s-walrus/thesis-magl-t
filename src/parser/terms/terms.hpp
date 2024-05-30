#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <boost/variant.hpp>

#include <functions/type.hpp>
#include <value/value.hpp>

namespace magl::parser::terms {

// TODO: Consider not storing a type with each term

/// Constant value
struct ValueTerm {
  value::Value value;

  functions::Type type;
};

/// Used in lambda definition
struct VariableTerm {
  size_t uid;

  functions::Type type;
};

/// Identifies an implementation in a FunctionLibrary
struct FunctionTerm {
  std::string name;

  functions::Type type;
};

/// Defines a reusable function
struct LambdaTerm;

/// Evaluation point of a function or a lambda
struct ApplicationTerm;

using Term = boost::variant<VariableTerm, boost::recursive_wrapper<LambdaTerm>,
                            boost::recursive_wrapper<ApplicationTerm>,
                            FunctionTerm, ValueTerm>;

struct LambdaTerm {
  VariableTerm argument;
  Term body;

  functions::Type type;
};

struct ApplicationTerm {
  // Possible values:
  // - function
  // - lambda
  // - value with an executable
  // - application that returns any of these
  // - variable that refers to any of these
  Term executable;

  std::vector<Term> arguments;

  functions::Type type;
};

}  // namespace magl::parser::terms
