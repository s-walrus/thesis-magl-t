#include <set>

#include <parser/syntax/functions.hpp>
#include <parser/utils/overloaded.hpp>

namespace magl::parser::syntax {

static const std::set<std::string> functions = {
    "Add",        //
    "Subtract",   //
    "Multiply",   //
    "Divide",     //
    "Or",         //
    "And",        //
    "Map",        //
    "ToString",   //
    "GetAnyOne",  //
    "VarMapAt",   //
    "GetVar",     //
};

std::optional<FunctionTerm> GetFunction(
    const tokenizer::tokens::NameToken& in) {
  if (functions.contains(in.value)) {
    return FunctionTerm{in.value};
  }

  return std::nullopt;
}

std::optional<FunctionTerm> GetInfixFunction(
    const tokenizer::tokens::Token& in) {
  using namespace tokenizer::tokens;
  using Opt = std::optional<FunctionTerm>;

  return boost::apply_visitor(
      utils::overloaded{
          [](const auto&) -> Opt { return std::nullopt; },
          [](const PlusToken&) -> Opt { return GetFunction({"Add"}); },
          [](const MinusToken&) -> Opt { return GetFunction({"Subtract"}); },
          [](const MultToken&) -> Opt { return GetFunction({"Multiply"}); },
          [](const DivToken&) -> Opt { return GetFunction({"Divide"}); },
          [](const OrToken&) -> Opt { return GetFunction({"Or"}); },
          [](const AndToken&) -> Opt { return GetFunction({"And"}); },
      },
      in);
}

}  // namespace magl::parser::syntax
