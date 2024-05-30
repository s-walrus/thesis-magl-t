#include <benchmark/benchmark.h>

// MAGL
#include <executer/expression.hpp>
#include <parser/parser.hpp>

#include <fstream>
#include <iostream>
#include <optional>

namespace magl {
namespace {

std::optional<executer::Expression> expression;

void Setup(const ::benchmark::State & /*state*/) {
  expression.emplace(parser::Parse(R"EOF(
    {"result": Map(
        lambda x: {
                "value1": VarMapAt(x, "v1"),
                "value2": VarMapAt(x, "v2"),
                "value3": VarMapAt(x, "v3"),
                "value4": VarMapAt(x, "v4")
            },
        [
            {"v1": "abacaba", "v2": "abacaba", "v3": "abacaba", "v4": "abacaba"},
            {"v1": "abacaba", "v2": "abacaba", "v3": "abacaba", "v4": "abacaba"},
            {"v1": "abacaba", "v2": "abacaba", "v3": "abacaba", "v4": "abacaba"},
            {"v1": "abacaba", "v2": "abacaba", "v3": "abacaba", "v4": "abacaba"},
            {"v1": "abacaba", "v2": "abacaba", "v3": "abacaba", "v4": "abacaba"},
            {"v1": "abacaba", "v2": "abacaba", "v3": "abacaba", "v4": "abacaba"},
            {"v1": "abacaba", "v2": "abacaba", "v3": "abacaba", "v4": "abacaba"},
            {"v1": "abacaba", "v2": "abacaba", "v3": "abacaba", "v4": "abacaba"}
        ]
    )}
    )EOF"));
}

void Teardown(const ::benchmark::State & /*state*/) {}

void BenchmarkMaglMap(::benchmark::State &state) {
  executer::Expression *ex = &*expression;
  engine::RunStandalone([&] {
    for (auto _ : state) {
      auto value = ex->Evaluate({});
      ::benchmark::DoNotOptimize(value);
    }
  });
}

BENCHMARK(BenchmarkMaglMap)->Setup(Setup)->Teardown(Teardown)->MinWarmUpTime(1);

} // namespace
} // namespace magl
