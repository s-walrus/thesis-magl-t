#pragma once

#include <parser/tokenizer/tokens.hpp>

#include <sstream>
#include <string>

namespace magl::parser::tokenizer {

struct ParsingError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class Tokenizer {
 public:
  using Token = tokens::Token;

  Tokenizer(std::istream& is) : input_(is) {}

  Token NextToken();

  bool IsEnd() const;
  operator bool() const { return IsEnd(); }

 private:
  size_t SkipSpaces();

  [[noreturn]] void ThrowParsingError() const;

  unsigned char NextChar();

 private:
  std::istream& input_;

  size_t chars_read_ = 0;
  bool is_eof_ = false;
};

Tokenizer& operator>>(Tokenizer& tokenizer, tokens::Token& out);

}  // namespace magl::parser::tokenizer
