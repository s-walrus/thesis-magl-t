#pragma once

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <functions/type.hpp>
#include <parser/syntax/syntax-tree.hpp>

// Type-resolved syntax tree is mirrored from parser/terms/syntax-tree.hpp

namespace magl::parser::terms::inference {

struct FundamentalTerm {
  functions::Type type;
  syntax::FundamentalTerm value;
};

struct VariableTerm {
  functions::Type type;
  std::string name;
};

struct FunctionTerm {
  functions::Type type;
  std::string name;
};

struct LambdaTerm;
struct ApplicationTerm;
struct ArrayTerm;
struct ObjectTerm;

using TypeResolvedTerm =
    boost::variant<VariableTerm, boost::recursive_wrapper<LambdaTerm>,
                   boost::recursive_wrapper<ApplicationTerm>,
                   boost::recursive_wrapper<FunctionTerm>,
                   boost::recursive_wrapper<ArrayTerm>,
                   boost::recursive_wrapper<ObjectTerm>, FundamentalTerm>;

struct LambdaTerm {
  functions::Type type;
  std::string argument_name;
  TypeResolvedTerm body;
};

struct ApplicationTerm {
  functions::Type type;
  TypeResolvedTerm function;
  std::vector<TypeResolvedTerm> arguments;
};

struct ArrayTerm {
  functions::Type type;
  std::vector<TypeResolvedTerm> items;
};

struct ObjectTerm {
  functions::Type type;
  std::map<std::string, TypeResolvedTerm> items;
};

using TypeResolvedSyntaxTree = TypeResolvedTerm;

}  // namespace magl::parser::terms::inference
