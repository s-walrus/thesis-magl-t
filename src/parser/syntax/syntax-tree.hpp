#pragma once

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <boost/variant.hpp>

#include <functions/function-factory.hpp>

/**
 * t := Func
 *    | x
 *    | value
 *    | lambda x . t
 *    | t t
 *    | array(t, ..., t)
 *    | object(key: t, ..., key: t)
 * key := string
 * Func is one of predefined functions such as Map, Add, SchemaToDict
 * value := int | double | string | bool | none
 *
 * SyntaxTree := t
 */

namespace magl::parser::syntax {

// Fundamental terms

struct NullTerm {};

struct IntegerTerm {
  int64_t value;
};

struct DoubleTerm {
  double value;
};

struct StringTerm {
  std::string value;
};

struct BoolTerm {
  bool value;
};

using FundamentalTerm =
    boost::variant<NullTerm, IntegerTerm, DoubleTerm, StringTerm, BoolTerm>;

// Other terms

struct VariableTerm {
  std::string name;
};

struct FunctionTerm {
  std::string name;
};

struct LambdaTerm;
struct ApplicationTerm;
struct ArrayTerm;
struct ObjectTerm;

using Term =
    boost::variant<VariableTerm, boost::recursive_wrapper<LambdaTerm>,
                   boost::recursive_wrapper<ApplicationTerm>,
                   boost::recursive_wrapper<FunctionTerm>,
                   boost::recursive_wrapper<ArrayTerm>,
                   boost::recursive_wrapper<ObjectTerm>, FundamentalTerm>;

struct LambdaTerm {
  VariableTerm argument;
  Term body;
};

struct ApplicationTerm {
  Term function;
  std::vector<Term> arguments;
};

struct ArrayTerm {
  std::vector<Term> items;
};

struct ObjectTerm {
  std::map<std::string, Term> items;
};

using SyntaxTree = Term;

// Comparators

/*
bool operator==(const VariableTerm& lhs, const VariableTerm& rhs);
bool operator==(const LambdaTerm& lhs, const LambdaTerm& rhs);
bool operator==(const ApplicationTerm& lhs, const ApplicationTerm& rhs);
bool operator==(const FunctionTerm& lhs, const FunctionTerm& rhs);
bool operator==(const ArrayTerm& lhs, const ArrayTerm& rhs);
bool operator==(const ObjectTerm& lhs, const ObjectTerm& rhs);
bool operator==(const NullTerm& lhs, const NullTerm& rhs);
bool operator==(const IntTerm& lhs, const IntTerm& rhs);
bool operator==(const DoubleTerm& lhs, const DoubleTerm& rhs);
bool operator==(const StringTerm& lhs, const StringTerm& rhs);
bool operator==(const BoolTerm& lhs, const BoolTerm& rhs);

template <typename T>
bool operator==(const FundamentalTerm& lhs, const T& rhs) {
  return std::holds_alternative<T>(lhs) && std::get<T>(lhs) == rhs;
}

template <typename T>
bool operator==(const Term& lhs, const T& rhs) {
  return std::holds_alternative<T>(lhs) && std::get<T>(lhs) == rhs;
}
*/

}  // namespace magl::parser::syntax
