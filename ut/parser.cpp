#include <parser/parser.hpp>

#include <library/cpp/testing/gtest/gtest.h>

#include <cmath>

using namespace magl;
using namespace magl::parser::terms;

TEST(Parser, Value) {
  {
    parser::terms::SemanticGraph result = parser::Parse("42");
    EXPECT_TRUE(boost::get<parser::terms::ValueTerm>(result).type ==
                functions::Type{functions::IntegerType{}});
    EXPECT_TRUE(boost::get<parser::terms::ValueTerm>(result).value ==
                value::Value{value::IntegerValue{42}});
  }

  {
    parser::terms::SemanticGraph result = parser::Parse("3.14");
    EXPECT_TRUE(boost::get<parser::terms::ValueTerm>(result).type ==
                functions::Type{functions::DoubleType{}});
    EXPECT_TRUE(
        std::fabs(boost::get<value::FloatValue>(
                      boost::get<parser::terms::ValueTerm>(result).value) -
                  3.14) < 0.01);
  }

  {
    parser::terms::SemanticGraph result = parser::Parse("\"Hello World!\"");
    EXPECT_TRUE(boost::get<parser::terms::ValueTerm>(result).type ==
                functions::Type{functions::StringType{}});
    EXPECT_TRUE(boost::get<parser::terms::ValueTerm>(result).value ==
                value::Value{value::StringValue{"Hello World!"}});
  }

  {
    parser::terms::SemanticGraph result = parser::Parse("[1, 2, 3]");
    EXPECT_TRUE(boost::get<parser::terms::ValueTerm>(result).type ==
                functions::Type{functions::ListType{functions::IntegerType{}}});
    EXPECT_TRUE((boost::get<parser::terms::ValueTerm>(result).value ==
                 value::Value{value::ArrayValue{value::IntegerValue{1},
                                                value::IntegerValue{2},
                                                value::IntegerValue{3}}}));
  }

  {
    parser::terms::SemanticGraph result =
        parser::Parse("{\"a\": 1, \"b\": 2, \"c\": 3}");
    EXPECT_TRUE(
        (boost::get<parser::terms::ValueTerm>(result).type ==
             functions::Type{functions::DictType{functions::IntegerType{}}} ||
         boost::get<parser::terms::ValueTerm>(result).type ==
             functions::Type{
                 functions::SchemaType{{{"a", functions::IntegerType{}},
                                        {"b", functions::IntegerType{}},
                                        {"c", functions::IntegerType{}}}}}));
    EXPECT_TRUE(
        (boost::get<parser::terms::ValueTerm>(result).value ==
         value::Value{value::ObjectValue{{{"a", value::IntegerValue{1}},
                                          {"b", value::IntegerValue{2}},
                                          {"c", value::IntegerValue{3}}}}}));
  }
}

TEST(Parser, Arithmetics) {
  parser::terms::SemanticGraph result = parser::Parse("1 + 2 * 10");
  EXPECT_TRUE(boost::get<parser::terms::ApplicationTerm>(result).type ==
              functions::Type{functions::IntegerType{}});

  auto expected_add = boost::get<parser::terms::FunctionTerm>(
      boost::get<parser::terms::ApplicationTerm>(result).executable);
  EXPECT_TRUE(expected_add.name == "Add");
  EXPECT_TRUE((expected_add.type ==
               functions::Type{functions::FunctionType{
                   .argument = functions::IntegerType{},
                   .body = functions::FunctionType{functions::IntegerType{},
                                                   functions::IntegerType{}}}}))
      << functions::ToString(expected_add.type);

  EXPECT_EQ(boost::get<parser::terms::ApplicationTerm>(result).arguments.size(),
            2u);

  EXPECT_TRUE(
      boost::get<parser::terms::ValueTerm>(
          boost::get<parser::terms::ApplicationTerm>(result).arguments.at(0))
          .type == functions::Type{functions::IntegerType{}});
  EXPECT_TRUE(
      boost::get<parser::terms::ValueTerm>(
          boost::get<parser::terms::ApplicationTerm>(result).arguments.at(0))
          .value == value::Value{value::IntegerValue{1}});

  EXPECT_TRUE(
      boost::get<parser::terms::ApplicationTerm>(
          boost::get<parser::terms::ApplicationTerm>(result).arguments.at(1))
          .type == functions::Type{functions::IntegerType{}});
  auto rhs = boost::get<parser::terms::ApplicationTerm>(
      boost::get<parser::terms::ApplicationTerm>(result).arguments.at(1));

  auto expected_mult = boost::get<parser::terms::FunctionTerm>(rhs.executable);
  EXPECT_TRUE(expected_mult.name == "Multiply");
  EXPECT_TRUE((expected_add.type ==
               functions::Type{functions::FunctionType{
                   .argument = functions::IntegerType{},
                   .body = functions::FunctionType{functions::IntegerType{},
                                                   functions::IntegerType{}}}}))
      << functions::ToString(expected_add.type);

  EXPECT_EQ(rhs.arguments.size(), 2u);
  EXPECT_TRUE(boost::get<parser::terms::ValueTerm>(rhs.arguments.at(0)).type ==
              functions::Type{functions::IntegerType{}});
  EXPECT_TRUE(boost::get<parser::terms::ValueTerm>(rhs.arguments.at(0)).value ==
              value::Value{value::IntegerValue{2}});
  EXPECT_TRUE(boost::get<parser::terms::ValueTerm>(rhs.arguments.at(1)).type ==
              functions::Type{functions::IntegerType{}});
  EXPECT_TRUE(boost::get<parser::terms::ValueTerm>(rhs.arguments.at(1)).value ==
              value::Value{value::IntegerValue{10}});
}

TEST(Parser, ObjectConstructor) {
  EXPECT_NO_THROW(parser::Parse("{\"a\": 1 + 2}"));
}

TEST(Parser, LambdaDefinition) {
  parser::terms::SemanticGraph result =
      parser::Parse("lambda x: GetVar(x) + 2");

  EXPECT_TRUE(boost::get<parser::terms::LambdaTerm>(&result));
  EXPECT_EQ(functions::ToString(boost::get<LambdaTerm>(result).type),
            "(INT) -> (INT)");
}

TEST(Parser, LambdaApplication) {
  EXPECT_NO_THROW(parser::Parse("(lambda x: GetVar(x) * 2)(10)"));
  EXPECT_NO_THROW(parser::Parse("(lambda x: GetVar(x))(10)"));
  EXPECT_NO_THROW(parser::Parse("(lambda x: GetVar(x) + GetVar(x))(10)"));
}
