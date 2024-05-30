#include <vector>

#include <parser/syntax/operator-presedence.hpp>
#include <parser/syntax/syntax-tree.hpp>
#include <parser/tokenizer/tokenizer.hpp>
#include <parser/utils/stream.hpp>

namespace magl::parser::syntax {

struct ParsingError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class SyntaxParser {
 public:
  using FunctionNames = std::vector<std::string>;

  SyntaxParser();

  SyntaxTree Parse(tokenizer::Tokenizer* in) const;

 private:
  using Stream =
      utils::PeekableStream<tokenizer::Tokenizer, tokenizer::tokens::Token>;

 private:
  Term NextNode(Stream* in, OperatorPresedence max_presedence) const;

  Term NextApplicationOrSimplestTerm(Stream* in) const;

  Term NextSimplestTerm(Stream* in) const;

  [[noreturn]] void ThrowParsingError(
      std::string message = "<no message>") const;

 private:
  // No state
};

}  // namespace magl::parser::syntax
