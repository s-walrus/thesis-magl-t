#include <parser/parser.hpp>

#include <executer/term.hpp>
#include <parser/syntax/parser.hpp>
#include <parser/terms/compiler.hpp>
#include <parser/tokenizer/tokenizer.hpp>

#include <iostream>
#include <sstream>

namespace magl::parser {

terms::SemanticGraph Parse(std::string_view code) {
  // FIXME: Unnecessary copy
  std::istringstream ss(std::string{code});
  tokenizer::Tokenizer tokenizer{ss};

  const syntax::SyntaxParser parser;
  const syntax::SyntaxTree syntax_tree = parser.Parse(&tokenizer);

  return terms::Compile(syntax_tree);
}

}  // namespace magl::parser
