#include <parser/terms/compiler.hpp>

#include <functions/include.hpp>
#include <functions/library/append.hpp>
#include <functions/utils/value-type.hpp>
#include <parser/terms/inference/inference.hpp>
#include <parser/utils/overloaded.hpp>

#include <algorithm>
#include <format>
#include <iostream>

namespace magl::parser::terms {

namespace {

inference::Grammar MakeGrammar(const functions::FunctionsLibrary& functions) {
  inference::Grammar result;
  for (const auto& [symbol, def] : functions) {
    result[symbol] = def->GetPolymorphicType();
  }
  return result;
}

class UIDGenerator {
 public:
  UIDGenerator() : next_id_(0) {}
  UIDGenerator(size_t start) : next_id_(start) {}

  size_t GetUniqueID() { return next_id_++; }

 private:
  size_t next_id_;
};

// FIXME: Consider specifying return types explicitly to avoid unexpected
// type casting
const auto kGetValueVisitor = utils::overloaded{
    [](const syntax::NullTerm&) -> value::Value { return value::NullValue{}; },
    [](const syntax::IntegerTerm& t) -> value::Value { return t.value; },
    [](const syntax::DoubleTerm& t) -> value::Value { return t.value; },
    [](const syntax::StringTerm& t) -> value::Value { return t.value; },
    [](const syntax::BoolTerm& t) -> value::Value { return t.value; },
};

class CompileVisitor : boost::static_visitor<Term> {
 public:
  CompileVisitor() {}

  result_type operator()(const inference::FundamentalTerm& t) {
    value::Value value = boost::apply_visitor(kGetValueVisitor, t.value);
    assert(functions::utils::GetType(value) == t.type);

    return ValueTerm{.value = std::move(value), .type = t.type};
  }

  result_type operator()(const inference::ArrayTerm& t) {
    assert(boost::get<functions::ListType>(&t.type));

    if (std::all_of(t.items.begin(), t.items.end(),
                    [](const inference::TypeResolvedTerm& t) -> bool {
                      return boost::get<inference::FundamentalTerm>(&t);
                    })) {
      // All items are fundamental
      std::vector<value::Value> items;
      items.reserve(t.items.size());
      for (const inference::TypeResolvedTerm& item : t.items) {
        items.emplace_back(boost::apply_visitor(
            kGetValueVisitor,
            boost::get<inference::FundamentalTerm>(item).value));
      }
      return ValueTerm{.value = value::ArrayValue{std::move(items)},
                       .type = t.type};
    }

    // TODO: Consider not using append for const arrays of const arrays and
    // objects

    Term result = ValueTerm{
        .value = value::ArrayValue{},
        .type = t.type,
    };

    for (const inference::TypeResolvedTerm& item : t.items) {
      result = ApplicationTerm{
          .executable =
              FunctionTerm{
                  .name = "Append",
                  .type =
                      functions::FunctionType{
                          .argument = t.type,
                          .body =
                              functions::FunctionType{
                                  boost::get<functions::ListType>(t.type)
                                      .value_type,
                                  t.type}}},
          .arguments = {std::move(result), boost::apply_visitor(*this, item)},
          .type = t.type,
      };
    }

    return result;
  }

  result_type operator()(const inference::ObjectTerm& t) {
    assert(boost::get<functions::DictType>(&t.type) ||
           boost::get<functions::SchemaType>(&t.type));

    if (boost::get<functions::DictType>(&t.type)) {
      if (std::all_of(
              t.items.begin(), t.items.end(), [](const auto& t) -> bool {
                return boost::get<inference::FundamentalTerm>(&t.second);
              })) {
        // All items are fundamental
        value::ObjectValue items;
        for (const auto& [key, value] : t.items) {
          items.emplace(
              key, boost::apply_visitor(
                       kGetValueVisitor,
                       boost::get<inference::FundamentalTerm>(value).value));
        }
        return ValueTerm{.value = value::ObjectValue{std::move(items)},
                         .type = t.type};
      }

      const functions::Type value_type =
          boost::get<functions::DictType>(t.type).value_type;

      Term result = ValueTerm{
          .value = value::ObjectValue{},
          .type = t.type,
      };

      for (const auto& [key, value] : t.items) {
        result = ApplicationTerm{
            .executable =
                FunctionTerm{
                    .name = "Insert",
                    .type =
                        functions::FunctionType{
                            t.type,
                            functions::FunctionType{
                                functions::StringType{},
                                functions::FunctionType{value_type, t.type}}}},
            .arguments =
                {
                    std::move(result),
                    ValueTerm{.value = {key}, .type = functions::StringType{}},
                    boost::apply_visitor(*this, value),
                },
            .type = t.type,
        };
      }

      return result;
    }

    // Not implemented
    throw std::logic_error("Schema constructors are not implemented.");
  }

  result_type operator()(const inference::VariableTerm& t) {
    const auto find_var = variables_.find(t.name);
    if (find_var == variables_.end()) {
      throw std::runtime_error(std::format("Undefined variable: {}", t.name));
    }

    return VariableTerm{.uid = find_var->second, .type = t.type};
  }

  result_type operator()(const inference::FunctionTerm& t) {
    return FunctionTerm{.name = t.name, .type = t.type};
  }

  result_type operator()(const inference::ApplicationTerm& t) {
    std::vector<Term> args;
    args.reserve(t.arguments.size());
    for (const inference::TypeResolvedTerm& arg : t.arguments) {
      args.emplace_back(boost::apply_visitor(*this, arg));
    }

    return ApplicationTerm{
        .executable = boost::apply_visitor(*this, t.function),
        .arguments = std::move(args),
        .type = t.type,
    };
  }

  result_type operator()(const inference::LambdaTerm& t) {
    const functions::FunctionType* lambda_type =
        boost::get<functions::FunctionType>(&t.type);
    if (!lambda_type) {
      throw std::runtime_error("Lambda type is not a type function.");
    }

    const ScopedVariable var{&variables_, t.argument_name,
                             variable_uid_gen_.GetUniqueID()};

    return LambdaTerm{
        .argument =
            VariableTerm{
                .uid = var.GetUID(),
                .type = lambda_type->argument,
            },
        .body = boost::apply_visitor(*this, t.body),
        .type = t.type,
    };
  }

 private:
  class ScopedVariable {
   public:
    ScopedVariable(std::unordered_map<std::string, size_t>* variables,
                   std::string name, size_t uid)
        : var_name_(std::move(name)), var_uid_(uid), variables_(variables) {
      auto find_var = variables_->find(var_name_);
      if (find_var != variables_->end()) {
        old_uid_ = find_var->second;
      }
      variables_->emplace_hint(find_var, var_name_, var_uid_);
    }

    const std::string& GetName() const { return var_name_; }

    size_t GetUID() const { return var_uid_; }

    ~ScopedVariable() {
      if (old_uid_) {
        variables_->emplace(var_name_, old_uid_.value());
      } else {
        variables_->erase(var_name_);
      }
    }

   private:
    const std::string var_name_;
    const size_t var_uid_;

    std::optional<size_t> old_uid_;
    std::unordered_map<std::string, size_t>* variables_;
  };

 private:
  UIDGenerator variable_uid_gen_;
  std::unordered_map<std::string /*name*/, size_t /*uid*/> variables_;
};

}  // namespace

SemanticGraph Compile(const syntax::SyntaxTree& syntax) {
  inference::Grammar grammar = MakeGrammar(functions::MakeDefaultLibrary());
  const inference::TypeResolvedSyntaxTree resolved_syntax =
      inference::InferTypes(syntax, std::move(grammar));

  return boost::apply_visitor(CompileVisitor{}, resolved_syntax);
}

}  // namespace magl::parser::terms
