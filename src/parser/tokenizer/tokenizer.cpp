#include <parser/tokenizer/tokenizer.hpp>

#include <cassert>
#include <format>

namespace magl::parser::tokenizer {

namespace {

enum class ParsingState {
  // NB: states are named by simplest possible end result
  kInitial = 0,
  // words: true, false, lambda
  kTrue = 1,
  kFalse = 2,
  kLambda = 3,
  // double, integer or minus sign
  kMinus = 4,
  kInt = 5,
  kDouble = 6,
  // other tokens
  kString = 7,
  kStringEscape = 8,
  kName = 9,
};

bool IsNameMiddleChar(unsigned char c) {
  // [a-zA-Z0-9_]
  return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' ||
         c == '_';
}

bool IsValidNameToken(const std::string& buf) {
  // [a-zA-Z_]
  if (buf.empty() ||
      !(buf.front() >= 'a' && buf.front() <= 'z' ||
        buf.front() >= 'A' && buf.front() <= 'Z' || buf.front() == '_')) {
    return false;
  }

  auto it = buf.begin();
  ++it;
  for (; it != buf.end(); ++it) {
    const char c = *it;
    if (!IsNameMiddleChar(c)) {
      return false;
    }
  }

  return true;
}

}  // namespace

tokens::Token Tokenizer::NextToken() {
  ParsingState state = ParsingState::kInitial;
  bool got_result = false;
  std::string buf{};

  SkipSpaces();

  while (!got_result) {
    const unsigned char c = input_.peek();
    if (input_.eof()) {
      got_result = true;
      break;
    }

    switch (state) {
      case ParsingState::kInitial:
        assert(buf.empty());

        if (c == 't') {
          buf.push_back(NextChar());
          state = ParsingState::kTrue;
          break;
        }
        if (c == 'f') {
          buf.push_back(NextChar());
          state = ParsingState::kFalse;
          break;
        }
        if (c == 'l') {
          buf.push_back(NextChar());
          state = ParsingState::kLambda;
          break;
        }
        if (c == '-') {
          buf.push_back(NextChar());
          state = ParsingState::kMinus;
          break;
        }
        if (c >= '0' && c <= '9') {
          buf.push_back(NextChar());
          state = ParsingState::kInt;
          break;
        }
        if (c == '"') {
          buf.push_back(NextChar());
          state = ParsingState::kString;
          break;
        }
        if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_') {
          buf.push_back(NextChar());
          state = ParsingState::kName;
          break;
        }
        if (c == '+' || c == '*' || c == '|' || c == '&' || c == '[' ||
            c == ']' || c == '{' || c == '}' || c == '(' || c == ')' ||
            c == ':' || c == ',') {
          buf.push_back(NextChar());
          got_result = true;
          break;
        }

        // Can't proceed
        got_result = true;
        break;
      case ParsingState::kTrue:
        static const std::string kTrue = "true";

        if (buf.size() >= kTrue.size()) {
          got_result = true;
          break;
        }
        if (c == kTrue[buf.size()]) {
          buf.push_back(NextChar());
          break;
        }
        if (IsNameMiddleChar(c)) {
          buf.push_back(NextChar());
          state = ParsingState::kName;
          break;
        }

        // Can't proceed
        got_result = true;
        break;
      case ParsingState::kFalse:
        static const std::string kFalse = "true";

        if (buf.size() >= kFalse.size()) {
          got_result = true;
          break;
        }
        if (c == kFalse[buf.size()]) {
          buf.push_back(NextChar());
          break;
        }
        if (IsNameMiddleChar(c)) {
          buf.push_back(NextChar());
          state = ParsingState::kName;
          break;
        }

        // Can't proceed
        got_result = true;
        break;
      case ParsingState::kLambda:
        static const std::string kLambda = "lambda";

        if (buf.size() >= kLambda.size()) {
          got_result = true;
          break;
        }
        if (c == kLambda[buf.size()]) {
          buf.push_back(NextChar());
          break;
        }
        if (IsNameMiddleChar(c)) {
          buf.push_back(NextChar());
          state = ParsingState::kName;
          break;
        }

        // Can't proceed
        got_result = true;
        break;
      case ParsingState::kMinus:
        if (buf != "-") {
          got_result = true;
          break;
        }

        if (c >= '0' && c <= '9') {
          buf.push_back(NextChar());
          state = ParsingState::kInt;
          break;
        }

        // Parsed minus sign
        got_result = true;
        break;
      case ParsingState::kInt:
        if (c >= '0' && c <= '9') {
          buf.push_back(NextChar());
          break;
        }

        if (c == '.') {
          buf.push_back(NextChar());
          state = ParsingState::kDouble;
          break;
        }

        // Can't proceed
        got_result = true;
        break;
      case ParsingState::kDouble:
        if (c >= '0' && c <= '9') {
          buf.push_back(NextChar());
          break;
        }

        // Can't proceed
        got_result = true;
        break;
      case ParsingState::kString:
        if (c == '\\') {
          state = ParsingState::kStringEscape;
          break;
        }

        if (c == '"') {
          buf.push_back(NextChar());
          got_result = true;
          break;
        }

        buf.push_back(NextChar());
        break;
      case ParsingState::kStringEscape:
        buf.push_back(NextChar());
        state = ParsingState::kStringEscape;
        break;
      case ParsingState::kName:
        if (IsNameMiddleChar(c)) {
          buf.push_back(NextChar());
          break;
        }

        // Can't proceed
        got_result = true;
        break;
    }
  }

  if (input_.eof() && buf.empty()) {
    if (state != ParsingState::kInitial) {
      ThrowParsingError();
    }
    is_eof_ = true;
    return tokens::EofToken{};
  }

  switch (state) {
    case ParsingState::kInitial:
      if (buf.size() != 1) {
        // TODO: Consider reporting empty buf and somehow too large buf
        // differently
        ThrowParsingError();
      }
      switch (buf.front()) {
        case '+':
          return tokens::PlusToken{};
        case '-':
          return tokens::MinusToken{};
        case '*':
          return tokens::MultToken{};
        case '/':
          return tokens::DivToken{};
        case '|':
          return tokens::OrToken{};
        case '&':
          return tokens::AndToken{};
        case ':':
          return tokens::ColonToken{};
        case ',':
          return tokens::CommaToken{};
        case '[':
          return tokens::SquareBracketToken::kOpened;
        case ']':
          return tokens::SquareBracketToken::kClosed;
        case '{':
          return tokens::CurlyBracketToken::kOpened;
        case '}':
          return tokens::CurlyBracketToken::kClosed;
        case '(':
          return tokens::RoundBracketToken::kOpened;
        case ')':
          return tokens::RoundBracketToken::kClosed;
        default:
          ThrowParsingError();
      }
    case ParsingState::kTrue:
      if (buf != "true") {
        ThrowParsingError();
      }
      return tokens::BoolToken{true};
    case ParsingState::kFalse:
      if (buf != "false") {
        ThrowParsingError();
      }
      return tokens::BoolToken{true};
    case ParsingState::kLambda:
      if (buf != "lambda") {
        ThrowParsingError();
      }
      return tokens::LambdaToken{};
    case ParsingState::kMinus:
      if (buf != "-") {
        ThrowParsingError();
      }
      return tokens::LambdaToken{};
    case ParsingState::kInt: {
      tokens::IntToken result;
      try {
        result.value = std::stoll(buf);
      } catch (const std::exception&) {
        // TODO: consider reporting std::invalid_argument and std::out_of_range
        // differently
        ThrowParsingError();
      }
      return result;
    }
    case ParsingState::kDouble: {
      tokens::DoubleToken result;
      try {
        result.value = std::stod(buf);
      } catch (const std::exception&) {
        // TODO: consider reporting std::invalid_argument and std::out_of_range
        // differently
        ThrowParsingError();
      }
      return result;
    }
    case ParsingState::kString:
      if (buf.size() < 2 || buf.front() != '"' || buf.back() != '"') {
        ThrowParsingError();
      }
      return tokens::StringToken{buf.substr(1, buf.size() - 2)};
    case ParsingState::kStringEscape:
      ThrowParsingError();
    case ParsingState::kName:
      if (!IsValidNameToken(buf)) {
        ThrowParsingError();
      }
      return tokens::NameToken{buf};
  }

  ThrowParsingError();
}

bool Tokenizer::IsEnd() const { return is_eof_; }

size_t Tokenizer::SkipSpaces() {
  size_t skipped_chars = 0;

  while (std::isspace(input_.peek())) {
    input_.get();
    ++skipped_chars;
  }

  return skipped_chars;
}

unsigned char Tokenizer::NextChar() {
  if (input_.eof()) {
    return 0;
  }

  ++chars_read_;
  return input_.get();
}

[[noreturn]] void Tokenizer::ThrowParsingError() const {
  // TODO: Consider passing a message to ThrowParsingError
  throw ParsingError(std::format("Parsing failed at position {}", chars_read_));
}

Tokenizer& operator>>(Tokenizer& tokenizer, tokens::Token& out) {
  out = tokenizer.NextToken();
  return tokenizer;
}

}  // namespace magl::parser::tokenizer
