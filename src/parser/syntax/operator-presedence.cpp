#include <parser/syntax/operator-presedence.hpp>

#include <parser/utils/overloaded.hpp>

namespace magl::parser::syntax {

std::optional<OperatorPresedence> GetOperatorPresedence(
    const tokenizer::tokens::Token& op) {
  using namespace tokenizer::tokens;
  using Opt = std::optional<OperatorPresedence>;

  return boost::apply_visitor(
      utils::overloaded{
          [](const auto&) { return Opt{std::nullopt}; },
          [](const PlusToken&) { return Opt{OperatorPresedence::kAdditive}; },
          [](const MinusToken&) { return Opt{OperatorPresedence::kAdditive}; },
          [](const MultToken&) {
            return Opt{OperatorPresedence::kMultiplicative};
          },
          [](const DivToken&) {
            return Opt{OperatorPresedence::kMultiplicative};
          },
          [](const OrToken&) { return Opt{OperatorPresedence::kOr}; },
          [](const AndToken&) { return Opt{OperatorPresedence::kAnd}; },
          [](const CurlyBracketToken& t) -> Opt {
            if (t == CurlyBracketToken::kClosed) {
              return OperatorPresedence::kPunctuation;
            }
            return std::nullopt;
          },
          [](const SquareBracketToken& t) -> Opt {
            if (t == SquareBracketToken::kClosed) {
              return OperatorPresedence::kPunctuation;
            }
            return std::nullopt;
          },
          [](const RoundBracketToken& t) -> Opt {
            if (t == RoundBracketToken::kClosed) {
              return OperatorPresedence::kPunctuation;
            }
            return std::nullopt;
          },
          [](const CommaToken&) -> Opt {
            return OperatorPresedence::kPunctuation;
          },
          [](const EofToken&) -> Opt {
            return OperatorPresedence::kPunctuation;
          },
      },
      op);
}

}  // namespace magl::parser::syntax
