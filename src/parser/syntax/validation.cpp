#include <parser/syntax/validation.hpp>

namespace magl::parser::syntax {

bool IsValidVariableName(const std::string& in) {
  static const std::regex kVariableNameRegex{"[a-z_][a-z_A-Z0-9]*"};

  return std::regex_match(in, kVariableNameRegex);
}

}  // namespace magl::parser::syntax
