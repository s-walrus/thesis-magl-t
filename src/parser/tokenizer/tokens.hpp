#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include <boost/variant.hpp>
#include <boost/variant/get.hpp>

// TODO: add `==` token

/**
 * Int: -?[0-9]+
 * Double: -?[0-9]+.[0-9]+
 * String: ".*?"
 * Bool: (true|false)
 * Name: [a-zA-Z_][a-zA-Z_0]*
 *
 *
 * Examples:
 *
 * xget("sources/my-source/code")
 * ^   ^^token                  ^token
 *
 * get_source("my-source").code
 * ^         ^^          ^^^
 *
 * Map(lambda x: ToSting(x), [-1, 2, 3000])
 * ^  ^^      ^^ ^      ^^^^ ^^ ^ ^^ ^   ^^
 */

namespace magl::parser::tokenizer::tokens {

struct IntToken {
  int64_t value;
};

struct DoubleToken {
  double value;
};

struct StringToken {
  std::string value;
};

struct BoolToken {
  bool value;
};

// Variable or function name
struct NameToken {
  std::string value;
};

struct PlusToken {};
struct MinusToken {};
struct MultToken {};
struct DivToken {};
struct OrToken {};
struct AndToken {};

enum class SquareBracketToken { kOpened = 0, kClosed = 1 };
enum class CurlyBracketToken { kOpened = 0, kClosed = 1 };
enum class RoundBracketToken { kOpened = 0, kClosed = 1 };

struct CommaToken {};
struct LambdaToken {};
struct ColonToken {};
// struct DotToken {};

struct EofToken {};

using Token = boost::variant<IntToken, DoubleToken, StringToken, NameToken,
                             PlusToken, MinusToken, MultToken, DivToken,
                             OrToken, AndToken, BoolToken, SquareBracketToken,
                             CurlyBracketToken, RoundBracketToken, CommaToken,
                             LambdaToken, ColonToken, EofToken>;

// Comparators

bool operator==(IntToken lhs, IntToken rhs);
bool operator==(DoubleToken lhs, DoubleToken rhs);
bool operator==(StringToken lhs, StringToken rhs);
bool operator==(BoolToken lhs, BoolToken rhs);
bool operator==(NameToken lhs, NameToken rhs);

bool operator==(PlusToken, PlusToken);
bool operator==(MinusToken, MinusToken);
bool operator==(MultToken, MultToken);
bool operator==(DivToken, DivToken);
bool operator==(OrToken, OrToken);
bool operator==(AndToken, AndToken);
bool operator==(CommaToken, CommaToken);
bool operator==(LambdaToken, LambdaToken);
bool operator==(ColonToken, ColonToken);
bool operator==(EofToken, EofToken);

/* template <typename T>
bool operator==(const Token& lhs, const T& rhs) {
  T* value_p = boost::get<T>(&lhs);
  if (value_p == nullptr) {
    return false;
  }
  return *value_p == rhs;
} */

std::ostream& operator<<(std::ostream& Str, IntToken const& v);
std::ostream& operator<<(std::ostream& Str, DoubleToken const& v);
std::ostream& operator<<(std::ostream& Str, StringToken const& v);
std::ostream& operator<<(std::ostream& Str, BoolToken const& v);
std::ostream& operator<<(std::ostream& Str, NameToken const& v);
std::ostream& operator<<(std::ostream& Str, PlusToken const& v);
std::ostream& operator<<(std::ostream& Str, MinusToken const& v);
std::ostream& operator<<(std::ostream& Str, DivToken const& v);
std::ostream& operator<<(std::ostream& Str, OrToken const& v);
std::ostream& operator<<(std::ostream& Str, AndToken const& v);
std::ostream& operator<<(std::ostream& Str, CommaToken const& v);
std::ostream& operator<<(std::ostream& Str, LambdaToken const& v);
std::ostream& operator<<(std::ostream& Str, ColonToken const& v);
std::ostream& operator<<(std::ostream& Str, EofToken const& v);

}  // namespace magl::parser::tokenizer::tokens
