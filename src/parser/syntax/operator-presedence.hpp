#include <optional>

#include <parser/tokenizer/tokens.hpp>

namespace magl::parser::syntax {

// Operators with smaller precedence are applied first

// According to C++ operator presedence
enum class OperatorPresedence {
  kMinPresedence = 0,
  kMultiplicative = 5,
  kAdditive = 6,
  kRelational = 9,
  kEquality = 10,
  kAnd = 14,
  kOr = 15,
  kBeforePunctuation = 16,
  kPunctuation = 17,
};

std::optional<OperatorPresedence> GetOperatorPresedence(
    const tokenizer::tokens::Token& op);

}  // namespace magl::parser::syntax
