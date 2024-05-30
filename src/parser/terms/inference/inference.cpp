#include <parser/terms/inference/inference.hpp>

#include <algorithm>
#include <boost/variant/static_visitor.hpp>
#include <format>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <parser/syntax/syntax-tree.hpp>
#include <parser/terms/inference/unification.hpp>
#include <parser/utils/overloaded.hpp>

namespace magl::parser::terms::inference {

namespace {

// For debug
[[maybe_unused]] std::string FormatSubstitution(
    const std::unordered_map<functions::TypeVariable, functions::Type> s) {
  std::ostringstream ss;
  ss << "Substutitions {\n";
  for (const auto& [key, value] : s) {
    ss << std::format("  {}: {}\n", key.uid, functions::ToString(value));
  }
  ss << "}";
  return ss.str();
}

functions::Type DereferenceVariable(
    const std::unordered_map<functions::TypeVariable, functions::Type>&
        substitution,
    const functions::TypeVariable& x) {
  functions::Type result = x;

  // iteratively follow type_variables in the substitution while possible
  functions::TypeVariable* ptr = 0;
  while ((ptr = boost::get<functions::TypeVariable>(&result)) &&
         substitution.contains(*ptr)) {
    result = substitution.at(*ptr);
  }

  return result;
}

class ApplySubstitutionVisitor : boost::static_visitor<functions::Type> {
 public:
  ApplySubstitutionVisitor(
      const std::unordered_map<functions::TypeVariable, functions::Type>&
          substitution)
      : substitution_(substitution) {}

  result_type operator()(functions::TypeVariable& tau) {
    functions::Type result = DereferenceVariable(substitution_, tau);
    if (boost::get<functions::TypeVariable>(&result) &&
        boost::get<functions::TypeVariable>(result).uid == tau.uid) {
      return result;
    }

    // Dereferencing produced a different type, applying substitution to it
    return boost::apply_visitor(*this, result);
  }
  result_type operator()(functions::FunctionType& tau) {
    tau.body = boost::apply_visitor(*this, tau.body);
    tau.argument = boost::apply_visitor(*this, tau.argument);
    return tau;
  }
  result_type operator()(functions::IntegerType& tau) { return tau; }
  result_type operator()(functions::DoubleType& tau) { return tau; }
  result_type operator()(functions::BoolType& tau) { return tau; }
  result_type operator()(functions::StringType& tau) { return tau; }
  result_type operator()(functions::NullType& tau) { return tau; }
  result_type operator()(functions::AnyType& tau) { return tau; }
  result_type operator()(functions::ListType& tau) {
    tau.value_type = boost::apply_visitor(*this, tau.value_type);
    return tau;
  }
  result_type operator()(functions::DictType& tau) {
    tau.value_type = boost::apply_visitor(*this, tau.value_type);
    return tau;
  }
  result_type operator()(functions::SchemaType& tau) {
    for (auto& [key, value] : tau.values) {
      value = boost::apply_visitor(*this, value);
    }
    return tau;
  }

 private:
  const std::unordered_map<functions::TypeVariable, functions::Type>&
      substitution_;
};

functions::Type ApplySubstitution(
    functions::Type type,
    const std::unordered_map<functions::TypeVariable, functions::Type>
        substitution) {
  return boost::apply_visitor(ApplySubstitutionVisitor{substitution}, type);
}

}  // namespace

class Environment {
 public:
  Environment(Grammar g) : next_id_(1000), grammar_(std::move(g)) {}

  std::size_t MakeUniqueID() { return next_id_++; }

  Grammar& GetGrammar() { return grammar_; }
  const Grammar& GetGrammar() const { return grammar_; }

 private:
  std::size_t next_id_;
  Grammar grammar_;
};

class InstantiatePolymorphicTypeVisitor
    : boost::static_visitor<functions::Type> {
 public:
  InstantiatePolymorphicTypeVisitor(
      Environment& env,
      const std::unordered_set<functions::TypeVariable>& non_generic,
      const std::unordered_map<functions::TypeVariable, functions::Type>&
          substitution)
      : env_(env),
        non_generic_variables_(non_generic),
        substitution_(substitution) {}

  result_type operator()(functions::TypeVariable var) {
    const functions::Type dereferenced =
        DereferenceVariable(substitution_, var);
    if (auto var_p = boost::get<functions::TypeVariable>(&dereferenced)) {
      var = *var_p;
    } else {
      return boost::apply_visitor(*this, dereferenced);
    }

    if (IsGeneric(var)) {
      if (!variable_mapping_.contains(var)) {
        variable_mapping_[var] = functions::TypeVariable{env_.MakeUniqueID()};
      }

      return variable_mapping_[var];
    }

    // Not generic
    return var;
  }

  result_type operator()(const functions::FunctionType& op) {
    return functions::FunctionType{boost::apply_visitor(*this, op.argument),
                                   boost::apply_visitor(*this, op.body)};
  }

  result_type operator()(const functions::IntegerType& op) { return op; }
  result_type operator()(const functions::DoubleType& op) { return op; }
  result_type operator()(const functions::BoolType& op) { return op; }
  result_type operator()(const functions::StringType& op) { return op; }
  result_type operator()(const functions::NullType& op) { return op; }
  result_type operator()(const functions::AnyType& op) { return op; }
  result_type operator()(const functions::ListType& op) {
    return functions::ListType{boost::apply_visitor(*this, op.value_type)};
  }
  result_type operator()(const functions::DictType& op) {
    return functions::DictType{boost::apply_visitor(*this, op.value_type)};
  }
  result_type operator()(const functions::SchemaType& op) {
    functions::SchemaType result;
    for (const auto& [key, value] : op.values) {
      result.values[key] = boost::apply_visitor(*this, value);
    }
    return result;
  }

 private:
  bool IsGeneric(const functions::TypeVariable& var) const {
    for (const auto& non_generic_var : non_generic_variables_) {
      functions::Type candidate = non_generic_var;
      if (ContainsVariable(candidate, var)) {
        return false;
      }

      functions::TypeVariable* ptr = nullptr;
      while ((ptr = boost::get<functions::TypeVariable>(&candidate)) &&
             substitution_.contains(*ptr)) {
        candidate = substitution_.at(*ptr);
        if (ContainsVariable(candidate, var)) {
          return false;
        }
      }
    }

    return true;
  }

  Environment& env_;
  const std::unordered_set<functions::TypeVariable>& non_generic_variables_;
  const std::unordered_map<functions::TypeVariable, functions::Type>&
      substitution_;
  std::unordered_map<functions::TypeVariable, functions::TypeVariable>
      variable_mapping_;
};

class ApplySubstitutionToSyntaxTreeVisitor : boost::static_visitor<void> {
 public:
  ApplySubstitutionToSyntaxTreeVisitor(
      const std::unordered_map<functions::TypeVariable, functions::Type>*
          substitution)
      : substitution_(substitution) {}

  result_type operator()(FundamentalTerm& t) {
    t.type =
        boost::apply_visitor(ApplySubstitutionVisitor{*substitution_}, t.type);
  }

  result_type operator()(ArrayTerm& t) {
    t.type =
        boost::apply_visitor(ApplySubstitutionVisitor{*substitution_}, t.type);
    for (TypeResolvedTerm& item : t.items) {
      boost::apply_visitor(*this, item);
    }
  }

  result_type operator()(ObjectTerm& t) {
    t.type =
        boost::apply_visitor(ApplySubstitutionVisitor{*substitution_}, t.type);
    for (auto& [key, value] : t.items) {
      boost::apply_visitor(*this, value);
    }
  }

  result_type operator()(VariableTerm& t) {
    t.type =
        boost::apply_visitor(ApplySubstitutionVisitor{*substitution_}, t.type);
  }

  result_type operator()(FunctionTerm& t) {
    t.type =
        boost::apply_visitor(ApplySubstitutionVisitor{*substitution_}, t.type);
  }

  result_type operator()(ApplicationTerm& t) {
    t.type =
        boost::apply_visitor(ApplySubstitutionVisitor{*substitution_}, t.type);
    boost::apply_visitor(*this, t.function);
    for (TypeResolvedTerm& arg : t.arguments) {
      boost::apply_visitor(*this, arg);
    }
  }

  result_type operator()(LambdaTerm& t) {
    t.type =
        boost::apply_visitor(ApplySubstitutionVisitor{*substitution_}, t.type);
    boost::apply_visitor(*this, t.body);
  }

 private:
  const std::unordered_map<functions::TypeVariable, functions::Type>*
      substitution_;
};

class InferVisitor
    : boost::static_visitor<std::pair<functions::Type, TypeResolvedTerm>> {
 public:
  InferVisitor(Environment env) : env_(env) {}

  result_type operator()(const syntax::FundamentalTerm& t) {
    const functions::Type result_type = boost::apply_visitor(
        utils::overloaded{
            [](const syntax::NullTerm&) -> functions::Type {
              return functions::NullType{};
            },
            [](const syntax::IntegerTerm&) -> functions::Type {
              return functions::IntegerType{};
            },
            [](const syntax::DoubleTerm&) -> functions::Type {
              return functions::DoubleType{};
            },
            [](const syntax::StringTerm&) -> functions::Type {
              return functions::StringType{};
            },
            [](const syntax::BoolTerm&) -> functions::Type {
              return functions::BoolType{};
            },
        },
        t);

    return {result_type, FundamentalTerm{.type = result_type, .value = t}};
  }

  result_type operator()(const syntax::ArrayTerm& t) {
    if (t.items.empty()) {
      // Empty list
      const functions::Type result =
          functions::ListType{functions::TypeVariable{env_.MakeUniqueID()}};
      return {result, ArrayTerm{.type = result, .items = {}}};
    }

    std::vector<functions::Type> argument_types;
    std::vector<TypeResolvedTerm> resolved_arguments;
    argument_types.reserve(t.items.size());
    resolved_arguments.reserve(t.items.size());

    // Infer types of the arguments
    for (size_t i = 0; i < t.items.size(); ++i) {
      const auto [item_type, resolved_item] = InferType(t.items[i]);
      argument_types.emplace_back(std::move(item_type));
      resolved_arguments.emplace_back(std::move(resolved_item));
    }

    // Determine unified argument type
    assert(!argument_types.empty());
    const functions::Type front_type = argument_types.front();
    for (size_t i = 1; i < t.items.size(); ++i) {
      try {
        // type(l[0]) = type(l[i])
        // FIXME: Substitutions may be broken after aborting with an exception
        Unify(front_type, argument_types[i], substitution_);
      } catch (const TypeMismatch&) {
        // List contains items of different types
        const functions::Type result_type =
            functions::ListType{functions::AnyType{}};
        return {result_type, ArrayTerm{.type = result_type,
                                       .items = std::move(resolved_arguments)}};
      }
      ApplySubstitution(front_type, substitution_);
    }

    const functions::Type result_type = functions::ListType{front_type};
    return {result_type, ArrayTerm{.type = result_type,
                                   .items = std::move(resolved_arguments)}};
  }

  result_type operator()(const syntax::ObjectTerm& t) {
    if (t.items.empty()) {
      // Empty dict-like
      // TODO: Return either a dict or a schema (undecided!)
      const functions::Type result =
          functions::DictType{functions::TypeVariable{env_.MakeUniqueID()}};
      return {result, ObjectTerm{.type = result, .items = {}}};
    }

    // TODO: Infer undecided dict|schema (or add a separate schema constructor)

    std::unordered_map<std::string, functions::Type> value_types;
    std::map<std::string, TypeResolvedTerm> resolved_values;
    value_types.reserve(t.items.size());

    // Infer types of the items
    for (const auto& [key, value] : t.items) {
      const auto [item_type, resolved_item] = InferType(value);
      value_types.emplace(key, std::move(item_type));
      resolved_values.emplace(key, std::move(resolved_item));
    }

    // Determine unified value type
    assert(!value_types.empty());
    const functions::Type front_type = value_types.begin()->second;
    for (auto it = ++t.items.begin(); it != t.items.end(); ++it) {
      try {
        // type(l[0]) = type(l[i])
        // FIXME: Substitutions may be broken after aborting with an exception
        Unify(front_type, value_types[it->first], substitution_);
      } catch (const TypeMismatch&) {
        // List contains items of different types
        // TODO: Infer undecided Dict[Any]|Schema[T1, T2, ..., Tn]
        const functions::Type result_type =
            functions::DictType{functions::AnyType{}};
        return {result_type, ObjectTerm{.type = result_type,
                                        .items = std::move(resolved_values)}};
      }
      ApplySubstitution(front_type, substitution_);
    }

    const functions::Type result_type = functions::DictType{front_type};
    return {result_type, ObjectTerm{.type = result_type,
                                    .items = std::move(resolved_values)}};
  }

  result_type operator()(const syntax::VariableTerm& t) {
    if (!env_.GetGrammar().contains(t.name)) {
      throw std::runtime_error(std::format("Underfined symbol {}", t.name));
    }

    // create a fresh type
    /* std::clog << "inferencer(VariableTerm): m_non_generic_variables: "
              << m_non_generic_variables << std::endl;
    std::clog << "inferencer(VariableTerm): calling fresh_maker on " << id.name
              << std::endl; */
    auto freshen_me = env_.GetGrammar()[t.name];
    auto v = InstantiatePolymorphicTypeVisitor(env_, non_generic_variables_,
                                               substitution_);
    const functions::Type variable_type = boost::apply_visitor(v, freshen_me);

    return {variable_type, VariableTerm{.type = variable_type, .name = t.name}};
  }

  result_type operator()(const syntax::FunctionTerm& t) {
    if (!env_.GetGrammar().contains(t.name)) {
      throw std::runtime_error(std::format("Underfined symbol {}", t.name));
    }

    // create a fresh type
    /* std::clog << "inferencer(VariableTerm): m_non_generic_variables: "
              << m_non_generic_variables << std::endl;
    std::clog << "inferencer(VariableTerm): calling fresh_maker on " << id.name
              << std::endl; */
    auto freshen_me = env_.GetGrammar()[t.name];
    auto v = InstantiatePolymorphicTypeVisitor{env_, non_generic_variables_,
                                               substitution_};
    const functions::Type function_type = boost::apply_visitor(v, freshen_me);

    return {function_type, FunctionTerm{.type = function_type, .name = t.name}};
  }

  result_type operator()(const syntax::ApplicationTerm& app) {
    /* std::clog << "inferencer(apply): m_non_generic_variables: " << std::endl;
    std::clog << m_non_generic_variables << std::endl; */

    auto [result_type, resolved_function] =
        boost::apply_visitor(*this, app.function);

    ApplicationTerm result;
    result.function = resolved_function;

    // Apply arguments sequentially
    for (const syntax::Term& arg : app.arguments) {
      auto [arg_type, resolved_arg] = boost::apply_visitor(*this, arg);
      result.arguments.push_back(resolved_arg);

      // std::clog << "inferencer(apply): calling unique_id" << std::endl;
      auto x = functions::TypeVariable{env_.MakeUniqueID()};
      auto lhs = functions::FunctionType{arg_type, x};

      // (arg_type -> x) = function_type
      Unify(lhs, result_type, substitution_);

      // return x
      result_type = DereferenceVariable(substitution_, x);
    }

    result.type = result_type;
    return {result_type, result};
  }

  result_type operator()(const syntax::LambdaTerm& lambda) {
    // std::clog << "inferencer(lambda): calling unique_id" << std::endl;
    functions::Type arg_type = functions::TypeVariable{env_.MakeUniqueID()};

    // introduce a scope with a non-generic variable
    auto s = ScopedVariable(non_generic_variables_, env_, lambda.argument.name,
                            boost::get<functions::TypeVariable>(arg_type));

    // Determine the type of the lambda function's body.
    /* std::clog << "inferencer(lambda): m_non_generic_variables: "
              << non_generic_variables_ << std::endl; */
    auto [body_type, resolved_body] = boost::apply_visitor(*this, lambda.body);
    arg_type = DereferenceVariable(
        substitution_, boost::get<functions::TypeVariable>(arg_type));

    // std::clog << "inferencer(lambda): calling unique_id" << std::endl;
    auto x = functions::TypeVariable{env_.MakeUniqueID()};
    // x =~= (arg_type -> body_type)
    Unify(x, functions::FunctionType{arg_type, body_type}, substitution_);

    result_type result = {
        DereferenceVariable(substitution_, x),
        LambdaTerm{
            .type = DereferenceVariable(substitution_, x),
            .argument_name = lambda.argument.name,
            .body = std::move(resolved_body),
        },
    };

    return result;
  }

  /// Non-recursive let
  /* result_type operator()(const syntax::let& let) {
    auto defn_type = boost::apply_visitor(*this, let.definition());

    // introduce a scope with a generic variable
    auto s = ScopedGenericVariable(this, let.name, defn_type);

    auto result = boost::apply_visitor(*this, let.body());

    return result;
  } */

  /// Recursive let
  /* result_type operator()(const syntax::letrec& letrec) {
    // std::clog << "inferencer(letrec): calling unique_id" << std::endl;
    auto new_type = functions::TypeVariable(env_.MakeUniqueID());

    // introduce a scope with a non generic variable
    auto s = ScopedVariable(this, letrec.name, new_type);

    auto definition_type = boost::apply_visitor(*this, letrec.definition());

    // new_type = definition_type
    Unify(new_type, definition_type, m_substitution);

    auto result = boost::apply_visitor(*this, letrec.body());

    return result;
  } */

  const std::unordered_map<functions::TypeVariable, functions::Type>&
  GetSubstitution() const {
    return substitution_;
  }

  class ScopedGenericVariable {
   public:
    ScopedGenericVariable(Environment& env, const std::string& variable_name,
                          functions::Type variable_type)
        : env_(env), variable_name_(variable_name) {
      auto find_type = env.GetGrammar().find(variable_name);
      if (find_type != env.GetGrammar().end()) {
        old_type_ = std::move(find_type->second);
      }
      env.GetGrammar().emplace_hint(find_type, variable_name,
                                    std::move(variable_type));
    }

    ~ScopedGenericVariable() {
      if (old_type_.has_value()) {
        env_.GetGrammar().emplace(variable_name_, std::move(old_type_.value()));
      } else {
        env_.GetGrammar().erase(variable_name_);
      }
    }

   private:
    Environment& env_;

    std::string variable_name_;
    std::optional<functions::Type> old_type_;
  };

  class ScopedVariable : ScopedGenericVariable {
   public:
    ScopedVariable(
        std::unordered_set<functions::TypeVariable>& non_generic_variables,
        Environment& env, const std::string& variable_name,
        functions::TypeVariable variable_type)
        : ScopedGenericVariable(env, variable_name, variable_type),
          non_generic_variables_(non_generic_variables) {
      const bool inserted = non_generic_variables_.insert(variable_type).second;
      if (inserted) {
        inserted_type_ = std::move(variable_type);
      }
    }

    ~ScopedVariable() {
      if (inserted_type_.has_value()) {
        non_generic_variables_.erase(inserted_type_.value());
      }
    }

   private:
    std::unordered_set<functions::TypeVariable>& non_generic_variables_;
    std::optional<functions::TypeVariable> inserted_type_;
  };

 private:
  std::pair<functions::Type, TypeResolvedTerm> InferType(
      const syntax::Term& t) {
    return boost::apply_visitor(*this, t);
  }

  bool IsGeneric(const functions::TypeVariable& var) const {
    for (const auto& non_generic_var : non_generic_variables_) {
      functions::Type candidate = non_generic_var;
      if (ContainsVariable(candidate, var)) {
        return false;
      }

      functions::TypeVariable* ptr = nullptr;
      while ((ptr = boost::get<functions::TypeVariable>(&candidate)) &&
             substitution_.contains(*ptr)) {
        candidate = substitution_.at(*ptr);
        if (ContainsVariable(candidate, var)) {
          return false;
        }
      }
    }

    return true;
  }

 private:
  Environment env_;

  std::unordered_set<functions::TypeVariable> non_generic_variables_;
  std::unordered_map<functions::TypeVariable, functions::Type> substitution_;
};

TypeResolvedSyntaxTree InferTypes(const syntax::Term& t, Grammar g) {
  Environment env{std::move(g)};
  InferVisitor inferer{std::move(env)};

  // Infer types
  TypeResolvedSyntaxTree result = boost::apply_visitor(inferer, t).second;

  // Apply substitution
  boost::apply_visitor(
      ApplySubstitutionToSyntaxTreeVisitor{&inferer.GetSubstitution()}, result);

  return result;
}

}  // namespace magl::parser::terms::inference
