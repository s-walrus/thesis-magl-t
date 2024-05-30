#include <parser/terms/inference/unification.hpp>

#include <boost/stacktrace.hpp>
#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace magl::parser::terms::inference {

typedef std::pair<functions::Type, functions::Type> Constraint;

namespace {

class ReplaceVariableVisitor : boost::static_visitor<functions::Type> {
 public:
  ReplaceVariableVisitor(const functions::TypeVariable& replace_me,
                         const functions::Type& replacement)
      : replace_me_(replace_me), replacement_(replacement) {}

  result_type operator()(functions::TypeVariable tau) {
    if (tau.uid == replace_me_.uid) {
      return replacement_;
    }
    return tau;
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
  const functions::TypeVariable& replace_me_;
  const functions::Type& replacement_;
};

class ContainsVariableVisitor : boost::static_visitor<bool> {
 public:
  ContainsVariableVisitor(const functions::TypeVariable& find_me)
      : find_me_(find_me) {}

  result_type operator()(const functions::TypeVariable tau) const {
    return tau.uid == find_me_.uid;
  }
  result_type operator()(const functions::FunctionType& tau) const {
    if (boost::apply_visitor(*this, tau.body)) {
      return true;
    }
    if (boost::apply_visitor(*this, tau.argument)) {
      return true;
    }
    return false;
  }
  result_type operator()(const functions::IntegerType&) const { return false; }
  result_type operator()(const functions::DoubleType&) const { return false; }
  result_type operator()(const functions::BoolType&) const { return false; }
  result_type operator()(const functions::StringType&) const { return false; }
  result_type operator()(const functions::NullType&) const { return false; }
  result_type operator()(const functions::AnyType&) const { return false; }
  result_type operator()(const functions::ListType& tau) const {
    return boost::apply_visitor(*this, tau.value_type);
  }
  result_type operator()(const functions::DictType& tau) const {
    return boost::apply_visitor(*this, tau.value_type);
  }
  result_type operator()(const functions::SchemaType& tau) const {
    for (auto& [key, value] : tau.values) {
      if (boost::apply_visitor(*this, value)) {
        return true;
      }
    }
    return false;
  }

 private:
  const functions::TypeVariable& find_me_;
};

class UnifyVisitor : public boost::static_visitor<void> {
 public:
  void operator()(const functions::TypeVariable& lhs,
                  const functions::Type& rhs) {
    if (const functions::TypeVariable* rhs_var =
            boost::get<functions::TypeVariable>(&rhs)) {
      // unify(var, var)
      if (lhs.uid != rhs_var->uid) {
        Eliminate(lhs, *rhs_var);
      }
    } else {
      // unify(var, t)
      if (ContainsVariable(rhs, lhs)) {
        throw RecursiveUnification(lhs, rhs);
      }

      Eliminate(lhs, rhs);
    }
  }

  void operator()(const functions::FunctionType& lhs,
                  const functions::Type& rhs) {
    if (boost::get<functions::TypeVariable>(&rhs)) {
      return boost::apply_visitor(*this, rhs, functions::Type{lhs});
    }

    if (!boost::get<functions::FunctionType>(&rhs)) {
      // Kind mismatch
      throw TypeMismatch(lhs, rhs, "* -> * != *");
    }

    const functions::FunctionType rhs_fun =
        boost::get<functions::FunctionType>(rhs);

    unification_stack_.push_back(std::make_pair(lhs.body, rhs_fun.body));

    unification_stack_.push_back(
        std::make_pair(lhs.argument, rhs_fun.argument));
  }

  void operator()(const functions::ListType& lhs, const functions::Type& rhs) {
    if (boost::get<functions::TypeVariable>(&rhs)) {
      return boost::apply_visitor(*this, rhs, functions::Type{lhs});
    }

    if (!boost::get<functions::ListType>(&rhs)) {
      throw TypeMismatch(lhs, rhs);
    }

    unification_stack_.push_back(std::make_pair(
        lhs.value_type, boost::get<functions::ListType>(rhs).value_type));
  }

  void operator()(const functions::DictType& lhs, const functions::Type& rhs) {
    if (boost::get<functions::TypeVariable>(&rhs)) {
      return boost::apply_visitor(*this, rhs, functions::Type{lhs});
    }

    if (!boost::get<functions::DictType>(&rhs)) {
      throw TypeMismatch(lhs, rhs);
    }

    unification_stack_.push_back(std::make_pair(
        lhs.value_type, boost::get<functions::DictType>(rhs).value_type));
  }

  void operator()(const functions::SchemaType& lhs,
                  const functions::Type& rhs) {
    if (boost::get<functions::TypeVariable>(&rhs)) {
      return boost::apply_visitor(*this, rhs, functions::Type{lhs});
    }

    if (!boost::get<functions::SchemaType>(&rhs)) {
      throw TypeMismatch(lhs, rhs);
    }

    const functions::SchemaType rhs_schema =
        boost::get<functions::SchemaType>(rhs);
    if (lhs.values.size() != rhs_schema.values.size()) {
      throw TypeMismatch(lhs, rhs);
    }

    for (auto lhs_val = lhs.values.begin(), rhs_val = rhs_schema.values.begin();
         lhs_val != lhs.values.end(); ++lhs_val, ++rhs_val) {
      unification_stack_.push_back(
          std::make_pair(lhs_val->second, rhs_val->second));
    }
  }

  void operator()(const functions::IntegerType& lhs,
                  const functions::Type& rhs) {
    if (boost::get<functions::TypeVariable>(&rhs)) {
      return boost::apply_visitor(*this, rhs, functions::Type{lhs});
    }
    if (!boost::get<functions::IntegerType>(&rhs)) {
      throw TypeMismatch(lhs, rhs, "Unification failed");
    }
  }

  void operator()(const functions::DoubleType& lhs,
                  const functions::Type& rhs) {
    if (boost::get<functions::TypeVariable>(&rhs)) {
      return boost::apply_visitor(*this, rhs, functions::Type{lhs});
    }
    if (!boost::get<functions::DoubleType>(&rhs)) {
      throw TypeMismatch(lhs, rhs);
    }
  }

  void operator()(const functions::BoolType& lhs, const functions::Type& rhs) {
    if (boost::get<functions::TypeVariable>(&rhs)) {
      return boost::apply_visitor(*this, rhs, functions::Type{lhs});
    }
    if (!boost::get<functions::BoolType>(&rhs)) {
      throw TypeMismatch(lhs, rhs);
    }
  }

  void operator()(const functions::StringType& lhs,
                  const functions::Type& rhs) {
    if (boost::get<functions::TypeVariable>(&rhs)) {
      return boost::apply_visitor(*this, rhs, functions::Type{lhs});
    }
    if (!boost::get<functions::StringType>(&rhs)) {
      throw TypeMismatch(lhs, rhs);
    }
  }

  void operator()(const functions::NullType& lhs, const functions::Type& rhs) {
    if (boost::get<functions::TypeVariable>(&rhs)) {
      return boost::apply_visitor(*this, rhs, functions::Type{lhs});
    }
    if (!boost::get<functions::NullType>(&rhs)) {
      throw TypeMismatch(lhs, rhs);
    }
  }

  void operator()(const functions::AnyType& lhs, const functions::Type& rhs) {
    if (boost::get<functions::TypeVariable>(&rhs)) {
      return boost::apply_visitor(*this, rhs, functions::Type{lhs});
    }
    if (!boost::get<functions::AnyType>(&rhs)) {
      throw TypeMismatch(lhs, rhs);
    }
  }

  UnifyVisitor(std::vector<Constraint> constraints,
               std::unordered_map<functions::TypeVariable, functions::Type>&
                   substitution)
      : unification_stack_(std::move(constraints)),
        substitution_(substitution) {
    // add the current substitution to the stack
    // XXX this step might be unnecessary
    /* unification_stack_.insert(unification_stack_.end(),
    m_substitution.begin(), m_substitution.end()); m_substitution.clear(); */
  }

  void Unify() {
    while (!unification_stack_.empty()) {
      functions::Type x = std::move(unification_stack_.back().first);
      functions::Type y = std::move(unification_stack_.back().second);
      unification_stack_.pop_back();

      boost::apply_visitor(*this, x, y);
    }
  }

 private:
  void Eliminate(const functions::TypeVariable& x, const functions::Type& y) {
    // replace all occurrances of x with y in the stack and the substitution
    if (const functions::TypeVariable* y_var =
            boost::get<functions::TypeVariable>(&y)) {
      if (y_var->uid == x.uid) {
        return;
      }

      if (y_var->uid < x.uid) {
        return Eliminate(*y_var, x);
      }
    }

    for (auto i = unification_stack_.begin(); i != unification_stack_.end();
         ++i) {
      ReplaceVariable(i->first, x, y);
      ReplaceVariable(i->second, x, y);
    }

    for (auto i = substitution_.begin(); i != substitution_.end(); ++i) {
      ReplaceVariable(i->second, x, y);
    }

    // add x = y to the substitution
    substitution_[x] = y;
  }

 private:
  std::vector<Constraint> unification_stack_;
  std::unordered_map<functions::TypeVariable, functions::Type>& substitution_;
};

}  // namespace

bool ContainsVariable(const functions::Type& haystack,
                      const functions::TypeVariable& needle) {
  return boost::apply_visitor(ContainsVariableVisitor{needle}, haystack);
}

void ReplaceVariable(functions::Type& t,
                     const functions::TypeVariable& replace_me,
                     const functions::Type& replacement) {
  t = boost::apply_visitor(ReplaceVariableVisitor{replace_me, replacement}, t);
}

template <typename Iterator>
void Unify(Iterator first_constraint, Iterator last_constraint,
           std::unordered_map<functions::TypeVariable, functions::Type>&
               substitution) {
  UnifyVisitor u({first_constraint, last_constraint}, substitution);
  u.Unify();
}

template <typename Range>
void Unify(const Range& rng,
           std::unordered_map<functions::TypeVariable, functions::Type>&
               substitution) {
  return Unify(rng.begin(), rng.end(), substitution);
}

void Unify(const functions::Type& x, const functions::Type& y,
           std::unordered_map<functions::TypeVariable, functions::Type>&
               substitution) {
  auto c = Constraint(x, y);
  return Unify(&c, &c + 1, substitution);
}

template <typename Range>
std::unordered_map<functions::TypeVariable, functions::Type> Unify(
    const Range& rng) {
  std::unordered_map<functions::TypeVariable, functions::Type> solutions;
  Unify(rng, solutions);
  return std::move(solutions);
}

}  // namespace magl::parser::terms::inference
