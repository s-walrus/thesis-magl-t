#pragma once

#include <executer/term.hpp>
#include <value/value.hpp>

namespace magl::executer {

functions::ValueHolder EvaluateTerm(EvaluationTree& t);

}  // namespace magl::executer
