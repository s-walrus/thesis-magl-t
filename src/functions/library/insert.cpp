#include <functions/library/insert.hpp>

#include <value/value.hpp>

#include <iostream>

namespace magl::functions::library {

// Dict[X] -> String -> X -> Dict[X]
const functions::Type Insert::kType = functions::FunctionType{
    functions::DictType{functions::TypeVariable{'X'}},
    functions::FunctionType{
        functions::StringType{},
        functions::FunctionType{
            functions::TypeVariable{'X'},
            functions::DictType{functions::TypeVariable{'X'}}}}};

namespace {

struct MakeInsertImplVisitor
    : boost::static_visitor<std::unique_ptr<IEvaluatable>> {
  MakeInsertImplVisitor() = default;

  result_type operator()(const functions::TypeVariable&) const {
    throw std::invalid_argument(
        "Insert: Dict[Var] -> String -> Var -> Dict[Var] is not "
        "supported.");
  }

  result_type operator()(const functions::FunctionType&) const {
    throw std::invalid_argument(
        "Insert: Dict[* -> *] -> String -> (* -> *) -> Dict[* -> *] is not "
        "supported.");
  }

  result_type operator()(const functions::IntegerType&) const {
    return std::make_unique<InsertImpl<value::IntegerValue>>();
  }

  result_type operator()(const functions::DoubleType&) const {
    return std::make_unique<InsertImpl<value::FloatValue>>();
  }

  result_type operator()(const functions::BoolType&) const {
    return std::make_unique<InsertImpl<value::BoolValue>>();
  }

  result_type operator()(const functions::StringType&) const {
    return std::make_unique<InsertImpl<value::StringValue>>();
  }

  result_type operator()(const functions::NullType&) const {
    return std::make_unique<InsertImpl<value::NullValue>>();
  }

  result_type operator()(const functions::AnyType&) const {
    throw std::invalid_argument(
        "Insert: Dict[Any] -> String -> Any -> Dict[Any] is not "
        "supported. TODO: add support.");
  }

  result_type operator()(const functions::ListType& /*tau*/) const {
    return std::make_unique<InsertImpl<value::ArrayValue>>();
  }

  result_type operator()(const functions::DictType& /*tau*/) const {
    return std::make_unique<InsertImpl<value::ObjectValue>>();
  }

  result_type operator()(const functions::SchemaType& /*tau*/) const {
    return std::make_unique<InsertImpl<value::ObjectValue>>();
  }
};

}  // namespace

std::unique_ptr<IEvaluatable> MakeInsertImpl(const functions::Type& x_type) {
  return boost::apply_visitor(MakeInsertImplVisitor{}, x_type);
}

std::unique_ptr<IEvaluatable> Insert::GetImplementation(
    const Type& specific_type) {
  auto mapping = utils::IsInstanceOf(specific_type, kType);
  if (!mapping) {
    throw UnsupportedType(
        std::format("Requested type does not match polymorphic type of Insert: "
                    "{} !~ {} (requested !~ polymorphic)",
                    ToString(specific_type), ToString(kType)));
  }

  const functions::Type& x_type = mapping.value().at({'X'});
  return MakeInsertImpl(x_type);
}

}  // namespace magl::functions::library
