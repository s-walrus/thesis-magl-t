#include <parser/tokenizer/tokenizer.hpp>

#include <library/cpp/testing/gtest/gtest.h>

#include <format>
#include <sstream>
#include <vector>

using namespace magl::parser::tokenizer;

/* template <typename T>
bool Equals(tokens::Token lhs, T rhs) {
  return std::holds_alternative<T>(lhs) && std::get<T>(lhs) == rhs;
} */

TEST(Tokenizer, Basic) {
  std::istringstream ss("Map(lambda x: ToString(x), [-1, 3.14, 3000])");

  Tokenizer t(ss);

  const std::vector<tokens::Token> expected_result = {
      tokens::NameToken{"Map"},
      tokens::RoundBracketToken::kOpened,
      tokens::LambdaToken{},
      tokens::NameToken{"x"},
      tokens::ColonToken{},
      tokens::NameToken{"ToString"},
      tokens::RoundBracketToken::kOpened,
      tokens::NameToken{"x"},
      tokens::RoundBracketToken::kClosed,
      tokens::CommaToken{},
      tokens::SquareBracketToken::kOpened,
      tokens::IntToken{-1},
      tokens::CommaToken{},
      tokens::DoubleToken{3.14},
      tokens::CommaToken{},
      tokens::IntToken{3000},
      tokens::SquareBracketToken::kClosed,
      tokens::RoundBracketToken::kClosed,
      tokens::EofToken{},
  };

  for (const tokens::Token& expected_token : expected_result) {
    EXPECT_TRUE(t.NextToken() == expected_token);
  }

  EXPECT_TRUE(t.IsEnd());
}

TEST(Tokenizer, Object) {
  std::istringstream ss(
      "{\"a\": \"b\", \"c\": {\"key\": [1, 2, 3]}, \"d\":-12}");

  Tokenizer t(ss);

  const std::vector<tokens::Token> expected_result = {
      tokens::CurlyBracketToken::kOpened,
      tokens::StringToken{"a"},
      tokens::ColonToken{},
      tokens::StringToken{"b"},
      tokens::CommaToken{},
      tokens::StringToken{"c"},
      tokens::ColonToken{},
      tokens::CurlyBracketToken::kOpened,
      tokens::StringToken{"key"},
      tokens::ColonToken{},
      tokens::SquareBracketToken::kOpened,
      tokens::IntToken{1},
      tokens::CommaToken{},
      tokens::IntToken{2},
      tokens::CommaToken{},
      tokens::IntToken{3},
      tokens::SquareBracketToken::kClosed,
      tokens::CurlyBracketToken::kClosed,
      tokens::CommaToken{},
      tokens::StringToken{"d"},
      tokens::ColonToken{},
      tokens::IntToken{-12},
      tokens::CurlyBracketToken::kClosed,
      tokens::EofToken{},
  };

  for (size_t i = 0; i < expected_result.size(); ++i) {
    EXPECT_TRUE(t.NextToken() == expected_result[i])
        << std::format("Unexpected token #{}", i);
  }

  EXPECT_TRUE(t.IsEnd());
}
