#include <executer/expression.hpp>
#include <parser/parser.hpp>

#include <library/cpp/testing/gtest/gtest.h>

using namespace magl;

TEST(Evaluation, Value) {
  {
    executer::Expression ex{parser::Parse("42")};
    EXPECT_TRUE(ex.Evaluate({}) == value::Value{value::IntegerValue{42}});
  }
}

TEST(Evaluation, Arithmetics) {
  {
    executer::Expression ex{parser::Parse("2 + 2")};
    EXPECT_TRUE(ex.Evaluate({}) == value::Value{value::IntegerValue{4}});
  }

  {
    executer::Expression ex{parser::Parse("2 * (11 + 10)")};
    EXPECT_EQ(boost::get<value::IntegerValue>(ex.Evaluate({})), 42);
  }
}

TEST(Evaluation, StaticObject) {
  {
    executer::Expression ex{parser::Parse("{\"key\": \"value\"}")};
    EXPECT_TRUE(
        (ex.Evaluate({}) == value::Value{value::ObjectValue{
                                {"key", value::StringValue{"value"}}}}));
  }
}

TEST(Evaluation, ObjectConstructor) {
  {
    executer::Expression ex{parser::Parse("{\"key\": 40 + 2}")};
    EXPECT_TRUE((ex.Evaluate({}) == value::Value{value::ObjectValue{
                                        {"key", value::IntegerValue{42}}}}));
  }
}

TEST(Evaluation, Lambda) {
  {
    executer::Expression ex{parser::Parse("(lambda x: GetVar(x))(10)")};
    EXPECT_TRUE((ex.Evaluate({}) == value::Value{value::IntegerValue(10)}));
  }

  {
    executer::Expression ex{
        parser::Parse("(lambda x: (GetVar(x) * GetVar(x)) + GetVar(x))(10)")};
    EXPECT_TRUE((ex.Evaluate({}) == value::Value{value::IntegerValue(110)}));
  }

  // TODO: Support void functions
  /* {
    executer::Expression ex{parser::Parse("(lambda x: 1000)()")};
    EXPECT_TRUE((ex.Evaluate({}) == value::Value{value::IntegerValue(1000)}));
  } */

  {
    executer::Expression ex{parser::Parse("(lambda x: GetVar(x) + 1)(1)")};
    EXPECT_TRUE((ex.Evaluate({}) == value::Value{value::IntegerValue(2)}));
  }
}

TEST(Evaluation, Debug) {
  {
    executer::Expression ex{parser::Parse("GetAnyOne()")};
    EXPECT_TRUE((ex.Evaluate({}) == value::Value{value::IntegerValue(1)}));
  }

  {
    executer::Expression ex{parser::Parse("GetAnyOne() + GetAnyOne()")};
    EXPECT_TRUE((ex.Evaluate({}) == value::Value{value::IntegerValue(2)}));
  }
}

TEST(Evaluation, Map) {
  executer::Expression ex{
      parser::Parse("Map(lambda x: GetVar(x) + 1, [1, 2, 3])")};
  auto result = ex.Evaluate({});
  EXPECT_TRUE(boost::get<value::ArrayValue>(&result));
  EXPECT_EQ(boost::get<value::ArrayValue>(result).size(), 3u);
  EXPECT_EQ(boost::get<value::IntegerValue>(
                boost::get<value::ArrayValue>(result).at(0)),
            2);
  EXPECT_EQ(boost::get<value::IntegerValue>(
                boost::get<value::ArrayValue>(result).at(1)),
            3);
  EXPECT_EQ(boost::get<value::IntegerValue>(
                boost::get<value::ArrayValue>(result).at(2)),
            4);
}

TEST(Evaluation, AdvancedMap) {
  executer::Expression ex{parser::Parse(R"EOF(
        Map(
            lambda x: {
                    "aaa": VarMapAt(x, "a") * 100,
                    "bbb": VarMapAt(x, "b") * 100
                },
            [
                {"a": 1, "b": 2},
                {"a": 3, "b": 4}
            ]
        )
    )EOF")};
  auto result = ex.Evaluate({});
  EXPECT_TRUE(
      (result == value::Value{value::ArrayValue{
                     value::ObjectValue{{"aaa", value::IntegerValue{100}},
                                        {"bbb", value::IntegerValue{200}}},
                     value::ObjectValue{{"aaa", value::IntegerValue{300}},
                                        {"bbb", value::IntegerValue{400}}}}}));
}
