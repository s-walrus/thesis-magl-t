#include <benchmark/benchmark.h>

// MAGL
#include <executer/expression.hpp>
#include <parser/parser.hpp>

#include <fstream>
#include <iostream>
#include <optional>

namespace magl::benchmark1 {
namespace {

std::optional<executer::Expression> expression;

void Setup(const ::benchmark::State & /*state*/) {
  expression.emplace(
      parser::Parse("(((1 + 2) + (3 + 4)) + ((5 + 6) + (7 + 8))) "
                    "+ (((9 + 10) + (11 + 12)) + ((13 + 14) + (15 + 16)))"));
}

void Teardown(const ::benchmark::State & /*state*/) {}

void BenchmarkMaglArithmetics(::benchmark::State &state) {
  executer::Expression *ex = &*expression;
  engine::RunStandalone([&] {
    for (auto _ : state) {
      auto value = ex->Evaluate({});
      ::benchmark::DoNotOptimize(value);
    }
  });
}

BENCHMARK(BenchmarkMaglArithmetics)
    ->Setup(Setup)
    ->Teardown(Teardown)
    ->MinWarmUpTime(1);

} // namespace
} // namespace magl::benchmark1

namespace magl::benchmark2 {
namespace {

std::optional<executer::Expression> expression;

void Setup(const ::benchmark::State & /*state*/) {
  expression.emplace(parser::Parse(
      "(((GetAnyOne() + GetAnyOne()) + (GetAnyOne() + GetAnyOne())) + "
      "((GetAnyOne() + GetAnyOne()) + (GetAnyOne() + GetAnyOne()))) "
      "+ (((GetAnyOne() + GetAnyOne()) + (GetAnyOne() + GetAnyOne())) + "
      "((GetAnyOne() + GetAnyOne()) + (GetAnyOne() + GetAnyOne())))"));
}

void Teardown(const ::benchmark::State & /*state*/) {}

void BenchmarkMaglArithmeticsNoInference(::benchmark::State &state) {
  executer::Expression *ex = &*expression;
  engine::RunStandalone([&] {
    for (auto _ : state) {
      auto value = ex->Evaluate({});
      ::benchmark::DoNotOptimize(value);
    }
  });
}

BENCHMARK(BenchmarkMaglArithmeticsNoInference)
    ->Setup(Setup)
    ->Teardown(Teardown)
    ->MinWarmUpTime(1);

} // namespace
} // namespace magl::benchmark2
