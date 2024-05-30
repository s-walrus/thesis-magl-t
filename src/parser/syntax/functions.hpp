#pragma once

#include <optional>

#include <parser/syntax/syntax-tree.hpp>
#include <parser/tokenizer/tokens.hpp>

namespace magl::parser::syntax {

std::optional<FunctionTerm> GetFunction(const tokenizer::tokens::NameToken& in);

std::optional<FunctionTerm> GetInfixFunction(
    const tokenizer::tokens::Token& in);

}  // namespace magl::parser::syntax
