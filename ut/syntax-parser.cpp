#include <parser/syntax/parser.hpp>

#include <sstream>
#include <vector>

#include <library/cpp/testing/gtest/gtest.h>

using namespace magl::parser::syntax;

namespace tokens = magl::parser::tokenizer::tokens;
namespace tokenizer = magl::parser::tokenizer;

TEST(SyntaxParser, Basic) {
  std::istringstream ss("Map(lambda x: ToString(x + 1), [-1, 0, 3000])");
  tokenizer::Tokenizer tokenizer{ss};

  const SyntaxParser parser;
  const SyntaxTree result = parser.Parse(&tokenizer);

  // Validate result

  EXPECT_TRUE(boost::get<ApplicationTerm>(&result));
  const ApplicationTerm& application = boost::get<ApplicationTerm>(result);

  EXPECT_TRUE(boost::get<FunctionTerm>(&application.function));
  const FunctionTerm& function = boost::get<FunctionTerm>(application.function);
  EXPECT_EQ(function.name, "Map");

  EXPECT_EQ(application.arguments.size(), 2ull);

  EXPECT_TRUE(boost::get<LambdaTerm>(&application.arguments[0]));
  const LambdaTerm& lambda = boost::get<LambdaTerm>(application.arguments[0]);
  EXPECT_EQ(lambda.argument.name, "x");

  EXPECT_TRUE(boost::get<ApplicationTerm>(&lambda.body));
  const ApplicationTerm& lam_body = boost::get<ApplicationTerm>(lambda.body);

  EXPECT_TRUE(boost::get<FunctionTerm>(&lam_body.function));
  const FunctionTerm& lam_body_function =
      boost::get<FunctionTerm>(lam_body.function);
  EXPECT_EQ(lam_body_function.name, "ToString");

  EXPECT_EQ(lam_body.arguments.size(), 1ull);
  EXPECT_TRUE(boost::get<ApplicationTerm>(&lam_body.arguments[0]));
  const ApplicationTerm& lam_body_func_arg =
      boost::get<ApplicationTerm>(lam_body.arguments[0]);

  EXPECT_TRUE(boost::get<FunctionTerm>(&lam_body_func_arg.function));
  const FunctionTerm& addition =
      boost::get<FunctionTerm>(lam_body_func_arg.function);
  EXPECT_EQ(addition.name, "Add");

  EXPECT_EQ(lam_body_func_arg.arguments.size(), 2ull);

  EXPECT_TRUE(boost::get<VariableTerm>(&lam_body_func_arg.arguments[0]));
  const VariableTerm& addition_lhs =
      boost::get<VariableTerm>(lam_body_func_arg.arguments[0]);
  EXPECT_EQ(addition_lhs.name, "x");

  EXPECT_TRUE(boost::get<FundamentalTerm>(&lam_body_func_arg.arguments[1]));
  EXPECT_TRUE(boost::get<IntegerTerm>(
      &boost::get<FundamentalTerm>(lam_body_func_arg.arguments[1])));
  const IntegerTerm& addition_rhs = boost::get<IntegerTerm>(
      boost::get<FundamentalTerm>(lam_body_func_arg.arguments[1]));
  EXPECT_EQ(addition_rhs.value, 1ll);

  EXPECT_TRUE(boost::get<ArrayTerm>(&application.arguments[1]));
  const ArrayTerm& array = boost::get<ArrayTerm>(application.arguments[1]);
  EXPECT_EQ(array.items.size(), 3ull);

  EXPECT_TRUE(boost::get<FundamentalTerm>(&array.items[0]));
  EXPECT_TRUE(
      boost::get<IntegerTerm>(&boost::get<FundamentalTerm>(array.items[0])));
  const IntegerTerm& array_first =
      boost::get<IntegerTerm>(boost::get<FundamentalTerm>(array.items[0]));
  EXPECT_EQ(array_first.value, -1ll);

  EXPECT_TRUE(boost::get<FundamentalTerm>(&array.items[1]));
  EXPECT_TRUE(
      boost::get<IntegerTerm>(&boost::get<FundamentalTerm>(array.items[0])));
  const IntegerTerm& array_second =
      boost::get<IntegerTerm>(boost::get<FundamentalTerm>(array.items[1]));
  EXPECT_EQ(array_second.value, 0ll);

  EXPECT_TRUE(boost::get<FundamentalTerm>(&array.items[2]));
  EXPECT_TRUE(
      boost::get<IntegerTerm>(&boost::get<FundamentalTerm>(array.items[0])));
  const IntegerTerm& array_third =
      boost::get<IntegerTerm>(boost::get<FundamentalTerm>(array.items[2]));
  EXPECT_EQ(array_third.value, 3000ll);
}

TEST(SyntaxParser, Object) {
  std::istringstream ss("{\"a\": \"b\", \"c\": -42}");
  tokenizer::Tokenizer tokenizer{ss};

  const SyntaxParser parser;
  const SyntaxTree result = parser.Parse(&tokenizer);

  // Validate result

  EXPECT_TRUE(boost::get<ObjectTerm>(&result));
  const ObjectTerm& object = boost::get<ObjectTerm>(result);

  EXPECT_EQ(object.items.size(), 2u);
  EXPECT_EQ(
      boost::get<StringTerm>(boost::get<FundamentalTerm>(object.items.at("a")))
          .value,
      "b");
  EXPECT_EQ(
      boost::get<IntegerTerm>(boost::get<FundamentalTerm>(object.items.at("c")))
          .value,
      -42);
}

TEST(SyntaxParser, Lambda) {
  std::istringstream ss("lambda x: x");
  tokenizer::Tokenizer tokenizer{ss};

  const SyntaxParser parser;
  const SyntaxTree result = parser.Parse(&tokenizer);

  // Validate result

  EXPECT_TRUE(boost::get<LambdaTerm>(&result));
  const LambdaTerm& lambda = boost::get<LambdaTerm>(result);

  EXPECT_TRUE(boost::get<VariableTerm>(&lambda.body));
  const VariableTerm& body = boost::get<VariableTerm>(lambda.body);

  EXPECT_EQ(body.name, lambda.argument.name);
}

TEST(SyntaxParser, LambdaInParenthesis) {
  std::istringstream ss("((((lambda x: x))))");
  tokenizer::Tokenizer tokenizer{ss};

  const SyntaxParser parser;
  const SyntaxTree result = parser.Parse(&tokenizer);

  // Validate result

  EXPECT_TRUE(boost::get<LambdaTerm>(&result));
  const LambdaTerm& lambda = boost::get<LambdaTerm>(result);

  EXPECT_TRUE(boost::get<VariableTerm>(&lambda.body));
  const VariableTerm& body = boost::get<VariableTerm>(lambda.body);

  EXPECT_EQ(body.name, lambda.argument.name);
}

TEST(SyntaxParser, LambdaApplication) {
  std::istringstream ss("(lambda x: x)(10)");
  tokenizer::Tokenizer tokenizer{ss};

  const SyntaxParser parser;
  const SyntaxTree result = parser.Parse(&tokenizer);

  // Validate result

  EXPECT_TRUE(boost::get<ApplicationTerm>(&result));
  const ApplicationTerm& application = boost::get<ApplicationTerm>(result);

  EXPECT_EQ(application.arguments.size(), 1u);
  EXPECT_TRUE(boost::get<FundamentalTerm>(&application.arguments.at(0)));

  EXPECT_TRUE(boost::get<LambdaTerm>(&application.function));
  const LambdaTerm& lambda = boost::get<LambdaTerm>(application.function);

  EXPECT_TRUE(boost::get<VariableTerm>(&lambda.body));
  const VariableTerm& body = boost::get<VariableTerm>(lambda.body);

  EXPECT_EQ(body.name, lambda.argument.name);
}
