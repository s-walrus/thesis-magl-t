#pragma once

#include <functions/type.hpp>

#include <unordered_map>

namespace magl::parser::terms::inference {

// Exceptions

struct TypeMismatch : std::runtime_error {
  TypeMismatch(const functions::Type& xx, const functions::Type& yy,
               std::string msg = "<no message>")
      : std::runtime_error(std::format(
            "Type mismatch: {} != {}  ({})", functions::ToString(xx),
            functions::ToString(yy), std::move(msg))),
        x(xx),
        y(yy) {}

  functions::Type x;
  functions::Type y;
};

struct RecursiveUnification : std::runtime_error {
  RecursiveUnification(const functions::Type& xx, const functions::Type& yy)
      : std::runtime_error(std::format("Recursive unification: {} != {}",
                                       functions::ToString(xx),
                                       functions::ToString(yy))),
        x(xx),
        y(yy) {}

  functions::Type x;
  functions::Type y;
};

// Functions

bool ContainsVariable(const functions::Type& haystack,
                      const functions::TypeVariable& needle);

void ReplaceVariable(functions::Type& t,
                     const functions::TypeVariable& replace_me,
                     const functions::Type& replacement);

void Unify(
    const functions::Type& x, const functions::Type& y,
    std::unordered_map<functions::TypeVariable, functions::Type>& substitution);

}  // namespace magl::parser::terms::inference
