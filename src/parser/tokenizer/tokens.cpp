#include <parser/tokenizer/tokens.hpp>

#include <ostream>

namespace magl::parser::tokenizer::tokens {

bool operator==(IntToken lhs, IntToken rhs) { return lhs.value == rhs.value; }
bool operator==(DoubleToken lhs, DoubleToken rhs) {
  return lhs.value == rhs.value;
}
bool operator==(StringToken lhs, StringToken rhs) {
  return lhs.value == rhs.value;
}
bool operator==(BoolToken lhs, BoolToken rhs) { return lhs.value == rhs.value; }
bool operator==(NameToken lhs, NameToken rhs) { return lhs.value == rhs.value; }

bool operator==(PlusToken, PlusToken) { return true; }
bool operator==(MinusToken, MinusToken) { return true; }
bool operator==(MultToken, MultToken) { return true; }
bool operator==(DivToken, DivToken) { return true; }
bool operator==(OrToken, OrToken) { return true; }
bool operator==(AndToken, AndToken) { return true; }
bool operator==(CommaToken, CommaToken) { return true; }
bool operator==(LambdaToken, LambdaToken) { return true; }
bool operator==(ColonToken, ColonToken) { return true; }
bool operator==(EofToken, EofToken) { return true; }

std::ostream& operator<<(std::ostream& Str, IntToken const& v) {
  return Str << "Int<" << v.value << ">";
}

std::ostream& operator<<(std::ostream& Str, DoubleToken const& v) {
  return Str << "Double<" << v.value << ">";
}

std::ostream& operator<<(std::ostream& Str, StringToken const& v) {
  return Str << "String<" << v.value << ">";
}

std::ostream& operator<<(std::ostream& Str, BoolToken const& v) {
  return Str << "Bool<" << (v.value ? "True" : "False") << ">";
}

std::ostream& operator<<(std::ostream& Str, NameToken const& v) {
  return Str << "Name<" << v.value << ">";
}

std::ostream& operator<<(std::ostream& Str, PlusToken const&) {
  return Str << "Plus";
}

std::ostream& operator<<(std::ostream& Str, MinusToken const&) {
  return Str << "Minus";
}

std::ostream& operator<<(std::ostream& Str, DivToken const&) {
  return Str << "Div";
}

std::ostream& operator<<(std::ostream& Str, OrToken const&) {
  return Str << "Or";
}

std::ostream& operator<<(std::ostream& Str, AndToken const&) {
  return Str << "And";
}

std::ostream& operator<<(std::ostream& Str, CommaToken const&) {
  return Str << "Comma";
}

std::ostream& operator<<(std::ostream& Str, LambdaToken const&) {
  return Str << "Lambda";
}

std::ostream& operator<<(std::ostream& Str, ColonToken const&) {
  return Str << "Colon";
}

std::ostream& operator<<(std::ostream& Str, EofToken const&) {
  return Str << "Eof";
}

}  // namespace magl::parser::tokenizer::tokens
