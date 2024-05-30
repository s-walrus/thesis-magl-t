#pragma once

#include <unordered_map>

#include <parser/syntax/syntax-tree.hpp>
#include <parser/terms/inference/syntax-tree-types.hpp>

namespace magl::parser::terms::inference {

using Grammar =
    std::unordered_map<std::string /*variable_name*/, functions::Type>;

TypeResolvedSyntaxTree InferTypes(const syntax::Term& t, Grammar g);

}  // namespace magl::parser::terms::inference
