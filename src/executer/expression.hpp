#pragma once

#include <executer/context.hpp>
#include <executer/term.hpp>
#include <parser/terms/semantic-graph.hpp>
#include <value/value.hpp>

namespace magl::executer {

struct ExpressionData {
  EvaluationTree term;
  functions::Type type;
};

class Expression {
 public:
  Expression(const parser::terms::SemanticGraph& graph);
  Expression(ExpressionData in)
      : term_(std::move(in.term)), type_(std::move(in.type)) {}

  // TODO: Make Evaluate const
  value::Value Evaluate(const EvaluationContext& context);

 private:
  EvaluationTree term_;
  functions::Type type_;
};

}  // namespace magl::executer
