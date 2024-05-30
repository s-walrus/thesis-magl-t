#pragma once

#include <vector>

#include <functions/evaluatable.hpp>
#include <functions/function-factory.hpp>

namespace magl::executer {

struct EvaluationTree {
  std::vector<EvaluationTree> args;
  std::unique_ptr<functions::IEvaluatable> implementation;
};

}  // namespace magl::executer
