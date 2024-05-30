#include <executer/expression.hpp>

#include <format>
#include <iostream>

#include <executer/evaluate.hpp>
#include <functions/include.hpp>
#include <functions/library/apply.hpp>
#include <functions/library/fetch-variable.hpp>
#include <functions/library/get-value.hpp>
#include <functions/library/lambda.hpp>
#include <functions/library/pass.hpp>

namespace magl::executer {

using namespace parser;

namespace {

/// Turns a term that stands for lhs in an application into an evaluatable
template <typename MakeExpressionVisitor>
class MakeApplicationVisitor
    : boost::static_visitor<std::unique_ptr<functions::IEvaluatable>> {
 public:
  MakeApplicationVisitor(MakeExpressionVisitor* parent,
                         functions::Type application_type)
      : parent_(parent), type_(application_type) {}

  result_type operator()(const parser::terms::VariableTerm& t) {
    const functions::FunctionType type =
        boost::get<functions::FunctionType>(type_);
    // TODO: Consider using single function fetch-and-apply. Will it improve
    // performance?
    return std::make_unique<functions::library::Apply>(
        std::make_unique<functions::library::FetchVariable>(
            parent_->variables_.at(t.uid)));
  }

  result_type operator()(const parser::terms::ApplicationTerm& /*t*/) {
    throw std::logic_error("Not implemented.");  // Not implemented
  }

  result_type operator()(const parser::terms::LambdaTerm& t) {
    // NB: Lambda recursion is prohibited

    // Save old variable location
    const std::optional<const functions::ValueHolder*> old_variable_location =
        [this, &t]() -> std::optional<const functions::ValueHolder*> {
      const auto find_variable = parent_->variables_.find(t.argument.uid);
      if (find_variable == parent_->variables_.end()) {
        return std::nullopt;
      }
      return find_variable->second;
    }();

    // Update variable location
    auto arg_holder = functions::library::Lambda::MakeVarHolder();
    parent_->variables_[t.argument.uid] = arg_holder.get();

    // Compile body
    ExpressionData compiled_body_data = boost::apply_visitor(*parent_, t.body);

    // Make lambda
    std::unique_ptr<functions::library::Lambda> lambda =
        std::make_unique<functions::library::Lambda>(
            std::move(arg_holder), std::move(compiled_body_data.term));

    // Revert variable location to its original value
    if (old_variable_location) {
      parent_->variables_[t.argument.uid] = old_variable_location.value();
    } else {
      parent_->variables_.erase(t.argument.uid);
    }

    return lambda;
  }

  result_type operator()(const parser::terms::FunctionTerm& t) {
    return parent_->functions_.at(t.name)->GetImplementation(t.type);
  }

  result_type operator()(const parser::terms::ValueTerm& /*t*/) {
    // TODO: Consider using IEvaluatable* as an evaluatable type in runtime
    // Not implemented
    throw std::logic_error(
        "Application with the function represented by a value is not "
        "implemented.");
    /*
      if (!boost::get<value::LambdaValue>(&t.value)) {
        throw std::runtime_error(
            "Function in an application is a non-evaluatable value.");
      }
      return boost::get<value::LambdaValue>(std::move(t.value));
    */
  }

 private:
  MakeExpressionVisitor* parent_;
  functions::Type type_;
};

class MakeExpressionVisitor : boost::static_visitor<ExpressionData> {
 public:
  MakeExpressionVisitor(functions::FunctionsLibrary functions)
      : functions_(std::move(functions)) {}

  result_type operator()(const parser::terms::VariableTerm& t) {
    const auto find_variable = variables_.find(t.uid);
    if (find_variable == variables_.end()) {
      throw std::runtime_error(
          std::format("Variable mapping is not defined for uid={}", t.uid));
    }
    return {
        .term =
            EvaluationTree{
                .args = {},
                .implementation =
                    std::make_unique<functions::library::FetchVariable>(
                        find_variable->second),
            },
        .type = t.type,
    };
  }

  result_type operator()(const parser::terms::ApplicationTerm& t) {
    std::vector<EvaluationTree> args;
    args.reserve(t.arguments.size());
    for (const terms::Term& arg : t.arguments) {
      args.emplace_back(boost::apply_visitor(*this, arg).term);
    }

    return {
        .term =
            {
                .args = std::move(args),
                .implementation =
                    boost::apply_visitor(MakeApplicationVisitor{this, t.type},
                                         terms::Term{t.executable}),
            },
        .type = t.type,
    };
  }

  result_type operator()(const parser::terms::LambdaTerm& t) {
    return ExpressionData{
        .term =
            {
                .args = {},
                .implementation =
                    std::make_unique<functions::library::GetLambdaValue>(
                        boost::apply_visitor(
                            MakeApplicationVisitor{this, t.type},
                            parser::terms::Term{t})),
            },
        .type = t.type,
    };
  }

  result_type operator()(const parser::terms::FunctionTerm& t) {
    return {
        EvaluationTree{
            .args = {},
            // TODO: Throw custom exception when function name is invalid
            .implementation =
                std::make_unique<functions::library::GetLambdaValue>(
                    functions_.at(t.name)->GetImplementation(t.type)),
        },
        t.type,
    };
  }

  result_type operator()(const parser::terms::ValueTerm& t) {
    return {
        EvaluationTree{
            .args = {},
            .implementation = functions::library::MakeGetValue(t.value)},
        functions::utils::GetType(t.value),
    };
  }

 private:
  friend class MakeApplicationVisitor<MakeExpressionVisitor>;

 private:
  functions::FunctionsLibrary functions_;

  std::unordered_map<size_t, const functions::ValueHolder*> variables_;
};

class GetValueVisitor : boost::static_visitor<value::Value> {
 public:
  GetValueVisitor(functions::ValueHolder* holder) : holder_(holder) {}

  result_type operator()(const functions::TypeVariable tau) const {
    throw std::invalid_argument(std::format(
        "Result value has unexpected type: Variable(uid={}).", tau.uid));
  }

  result_type operator()(const functions::FunctionType& /*tau*/) const {
    throw std::invalid_argument("Result value has unexpected type: Function.");
  }

  result_type operator()(const functions::IntegerType&) const {
    // TODO: Use memory-safe interface to ValueHolder
    auto* held_result = reinterpret_cast<value::IntegerValue*>(holder_);
    value::Value result = std::move(*held_result);
    return result;
  }

  result_type operator()(const functions::DoubleType&) const {
    // TODO: Use memory-safe interface to ValueHolder
    auto* held_result = reinterpret_cast<value::FloatValue*>(holder_);
    value::Value result = std::move(*held_result);
    return result;
  }

  result_type operator()(const functions::BoolType&) const {
    // TODO: Use memory-safe interface to ValueHolder
    auto* held_result = reinterpret_cast<value::FloatValue*>(holder_);
    value::Value result = std::move(*held_result);
    return result;
  }

  result_type operator()(const functions::StringType&) const {
    // TODO: Use memory-safe interface to ValueHolder
    auto* held_result = reinterpret_cast<value::StringValue*>(holder_);
    value::Value result = std::move(*held_result);
    // TODO: make prettier
    held_result->~basic_string();
    return result;
  }

  result_type operator()(const functions::NullType&) const {
    // TODO: Use memory-safe interface to ValueHolder
    auto* held_result = reinterpret_cast<value::NullValue*>(holder_);
    value::Value result = std::move(*held_result);
    return result;
  }

  result_type operator()(const functions::AnyType&) const {
    // TODO: Use memory-safe interface to ValueHolder
    // TODO: Check that values of AnyType are correctly put as a value::Value
    auto* held_result = reinterpret_cast<value::Value*>(holder_);
    value::Value result = std::move(*held_result);
    // TODO: held_result->~Value();
    return result;
  }

  result_type operator()(const functions::ListType& /*tau*/) const {
    // TODO: Use memory-safe interface to ValueHolder
    // TODO: If tau is list of T, interpret argument as std::vector<T> (if
    // direct array types are implemented). Memory-safe interfaces are needed
    auto* held_result = reinterpret_cast<value::ArrayValue*>(holder_);
    value::Value result = std::move(*held_result);
    // TODO: held_result->~Array();
    return result;
  }

  result_type operator()(const functions::DictType& /*tau*/) const {
    // TODO: Use memory-safe interface to ValueHolder
    // TODO: If tau is dict of T, interpret argument as map of T (if direct dict
    // types are implemented). Memory-safe interfaces are needed
    auto* held_result = reinterpret_cast<value::ObjectValue*>(holder_);
    value::Value result = std::move(*held_result);
    // TODO: held_result->~Object();
    return result;
  }

  result_type operator()(const functions::SchemaType& /*tau*/) const {
    // TODO: Use memory-safe interface to ValueHolder
    // TODO: Use direct value types if they are implemented
    auto* held_result = reinterpret_cast<value::ObjectValue*>(holder_);
    value::Value result = std::move(*held_result);
    // TODO: held_result->~Object();
    return result;
  }

 private:
  functions::ValueHolder* holder_;
};

// TODO: Use memory-safe interface to ValueHolder
value::Value GetValue(functions::ValueHolder* holder,
                      const functions::Type& result_type) {
  return boost::apply_visitor(GetValueVisitor{holder}, result_type);
}

}  // namespace

Expression::Expression(const parser::terms::SemanticGraph& graph)
    : Expression(boost::apply_visitor(
          MakeExpressionVisitor{functions::MakeDefaultLibrary()}, graph)) {}

value::Value Expression::Evaluate(const EvaluationContext& /*context*/) {
  functions::ValueHolder result = EvaluateTerm(term_);
  return GetValue(&result, type_);
}

}  // namespace magl::executer
