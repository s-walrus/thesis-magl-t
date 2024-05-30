#include <executer/evaluate.hpp>

namespace magl::executer {

// TODO: Optimize execution
// * consider avoiding recursion
// * consider reusing ArgsContainer considering cache hits
functions::ValueHolder EvaluateTerm(EvaluationTree& t) {
  functions::ArgsContainer args;
  functions::ValueHolder to;

  for (size_t i = 0; i < t.args.size(); ++i) {
    args[i] = EvaluateTerm(t.args[i]);
  }

  t.implementation->Evaluate(&args, &to);

  return to;
}

}  // namespace magl::executer
