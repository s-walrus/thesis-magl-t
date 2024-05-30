#pragma once

#include <parser/syntax/syntax-tree.hpp>
#include <parser/terms/semantic-graph.hpp>

namespace magl::parser::terms {

SemanticGraph Compile(const syntax::SyntaxTree& syntax);

}
