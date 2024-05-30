#include <parser/syntax/parser.hpp>

#include <cassert>
#include <format>

#include <parser/syntax/functions.hpp>
#include <parser/syntax/validation.hpp>
#include <parser/utils/overloaded.hpp>

namespace magl::parser::syntax {

namespace {

namespace tokens = tokenizer::tokens;

}  // namespace

SyntaxParser::SyntaxParser() {
  // Do nothing
}

SyntaxTree SyntaxParser::Parse(tokenizer::Tokenizer* in) const {
  Stream peekable_in{in};

  Term root = NextNode(&peekable_in, OperatorPresedence::kBeforePunctuation);

  const tokens::Token next_token = peekable_in.Next();
  if (next_token == tokens::Token{tokens::EofToken{}}) {
    return root;
  }

  ThrowParsingError();
}

Term SyntaxParser::NextNode(Stream* in,
                            OperatorPresedence max_presedence) const {
  Term root = NextApplicationOrSimplestTerm(in);

  while (true) {
    // Condition
    const auto pres = GetOperatorPresedence(*in->Peek());
    if (!pres.has_value() || pres.value() > max_presedence) {
      break;
    }

    const tokens::Token op = in->Next();
    Term rhs = NextNode(in, pres.value());

    ApplicationTerm new_root;
    new_root.function = GetInfixFunction(op).value();
    new_root.arguments.push_back(std::move(root));
    new_root.arguments.push_back(std::move(rhs));

    root = std::move(new_root);
  }

  return root;
}

Term SyntaxParser::NextApplicationOrSimplestTerm(Stream* in) const {
  if (GetOperatorPresedence(*in->Peek()).has_value()) {
    ThrowParsingError("Unexpected operator.");
  }

  Term root = NextSimplestTerm(in);

  if (*in->Peek() == tokens::Token{tokens::RoundBracketToken::kOpened}) {
    in->Next();
  } else {
    return root;
  }

  ApplicationTerm application;
  application.function = std::move(root);

  if (*in->Peek() != tokens::Token{tokens::RoundBracketToken::kClosed}) {
    // Non-empty argument list
    while (true) {
      application.arguments.push_back(
          NextNode(in, OperatorPresedence::kBeforePunctuation));

      // Condition
      if (boost::get<tokens::CommaToken>(in->Peek())) {
        in->Next();
      } else {
        break;
      }
    }
  }

  const tokens::Token closing_bracket = in->Next();
  if (closing_bracket != tokens::Token{tokens::RoundBracketToken::kClosed}) {
    ThrowParsingError(
        "Expected symbol ')' at the end of argument list, got something else.");
  }

  return application;
}

Term SyntaxParser::NextSimplestTerm(Stream* in) const {
  return boost::apply_visitor(
      utils::overloaded{
          // TODO: support unary minus operator
          [this](const auto&) -> Term {
            ThrowParsingError("Unexpected token.");
          },
          [in](const tokens::IntToken&) -> Term {
            return IntegerTerm{boost::get<tokens::IntToken>(in->Next()).value};
          },
          [in](const tokens::DoubleToken&) -> Term {
            return DoubleTerm{
                boost::get<tokens::DoubleToken>(in->Next()).value};
          },
          [in](const tokens::StringToken&) -> Term {
            return StringTerm{
                std::move(boost::get<tokens::StringToken>(in->Next())).value};
          },
          [in, this](const tokens::NameToken&) -> Term {
            const tokens::NameToken name =
                boost::get<tokens::NameToken>(in->Next());

            auto maybe_function = GetFunction(name);
            if (maybe_function.has_value()) {
              return Term{maybe_function.value()};
            }

            if (IsValidVariableName(name.value)) {
              return Term{VariableTerm{name.value}};
            }

            ThrowParsingError(
                std::format("Cannot interpret name symbol: {}", name.value));
          },
          [in](const tokens::BoolToken&) -> Term {
            return BoolTerm{boost::get<tokens::BoolToken>(in->Next()).value};
          },
          [in, this](const tokens::SquareBracketToken&) -> Term {
            const tokens::SquareBracketToken bracket =
                boost::get<tokens::SquareBracketToken>(in->Next());

            if (bracket == tokens::SquareBracketToken::kClosed) {
              ThrowParsingError("Unexpected symbol ']'.");
            }

            ArrayTerm result;
            while (true) {
              result.items.emplace_back(
                  NextNode(in, OperatorPresedence::kBeforePunctuation));

              // Condition
              if (boost::get<tokens::CommaToken>(in->Peek())) {
                in->Next();
              } else {
                break;
              }
            }

            const tokens::Token closing_bracket = in->Next();
            if (closing_bracket !=
                tokens::Token{tokens::SquareBracketToken::kClosed}) {
              ThrowParsingError("Expected symbol ']', got something else.");
            }

            return result;
          },
          [in, this](const tokens::CurlyBracketToken&) -> Term {
            const tokens::CurlyBracketToken bracket =
                boost::get<tokens::CurlyBracketToken>(in->Next());

            if (bracket == tokens::CurlyBracketToken::kClosed) {
              ThrowParsingError("Unexpected symbol: '}'");
            }

            ObjectTerm result;
            while (true) {
              tokens::Token key = in->Next();
              if (!boost::get<tokens::StringToken>(&key)) {
                ThrowParsingError("Expected string key, got not a string.");
              }
              std::string key_value =
                  std::move(boost::get<tokens::StringToken>(key).value);
              if (result.items.contains(key_value)) {
                ThrowParsingError(std::format(
                    "Duplicating keys in object definition: {}", key_value));
              }

              const tokens::Token colon = in->Next();
              if (!boost::get<tokens::ColonToken>(&colon)) {
                ThrowParsingError("Expected symbol ':', got something else.");
              }

              Term value = NextNode(in, OperatorPresedence::kBeforePunctuation);

              result.items[std::move(key_value)] = std::move(value);

              // Condition
              if (boost::get<tokens::CommaToken>(in->Peek())) {
                in->Next();
              } else {
                break;
              }
            }

            const tokens::Token closing_bracket = in->Next();
            if (closing_bracket !=
                tokens::Token{tokens::CurlyBracketToken::kClosed}) {
              ThrowParsingError("Expected symbol '}', got something else.");
            }

            return result;
          },
          [in, this](const tokens::RoundBracketToken&) -> Term {
            const tokens::RoundBracketToken bracket =
                boost::get<tokens::RoundBracketToken>(in->Next());

            if (bracket == tokens::RoundBracketToken::kClosed) {
              ThrowParsingError("Unexpected symbol ')'");
            }

            Term body = NextNode(in, OperatorPresedence::kBeforePunctuation);

            const tokens::Token closing_bracket = in->Next();
            if (closing_bracket !=
                tokens::Token{tokens::RoundBracketToken::kClosed}) {
              ThrowParsingError("Expected symbol ')', got something else.");
            }

            return body;
          },
          [this](const tokens::CommaToken&) -> Term {
            ThrowParsingError("Unexpected symbol ','");
          },
          [in, this](const tokens::LambdaToken&) -> Term {
            in->Next();

            LambdaTerm result;

            tokens::Token argument = in->Next();
            if (!boost::get<tokens::NameToken>(&argument)) {
              ThrowParsingError(
                  "Expected a name token for variable name, got something "
                  "else.");
            }
            if (!IsValidVariableName(
                    boost::get<tokens::NameToken>(argument).value)) {
              ThrowParsingError(
                  std::format("Invalid variable name: {}",
                              boost::get<tokens::NameToken>(argument).value));
            }
            result.argument = VariableTerm{
                std::move(boost::get<tokens::NameToken>(argument).value)};

            const tokens::Token colon = in->Next();
            if (!boost::get<tokens::ColonToken>(&colon)) {
              ThrowParsingError("Expected symbol ',', got something else.");
            }

            result.body = NextNode(in, OperatorPresedence::kBeforePunctuation);

            return result;
          },
          [this](const tokens::ColonToken&) -> Term {
            ThrowParsingError("Unexpected symbol ':'.");
          },
      },
      *in->Peek());
}

void SyntaxParser::ThrowParsingError(std::string message) const {
  // TODO: add position to the error message
  throw ParsingError(std::format("Parsing failed: {}", std::move(message)));
}

}  // namespace magl::parser::syntax
