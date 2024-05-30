#include <functions/library/append.hpp>

#include <value/value.hpp>

#include <iostream>

namespace magl::functions::library {

// List[X] -> X -> List[X]
const functions::Type Append::kType = functions::FunctionType{
    functions::ListType{functions::TypeVariable{'X'}},
    functions::FunctionType{functions::TypeVariable{'X'},
                            functions::ListType{functions::TypeVariable{'X'}}}};

namespace {

struct MakeAppendImplVisitor
    : boost::static_visitor<std::unique_ptr<IEvaluatable>> {
  MakeAppendImplVisitor() = default;

  result_type operator()(const functions::TypeVariable&) const {
    throw std::invalid_argument(
        "Append: Dict[Var] -> String -> Var -> Dict[Var] is not "
        "supported.");
  }

  result_type operator()(const functions::FunctionType&) const {
    throw std::invalid_argument(
        "Append: Dict[* -> *] -> String -> (* -> *) -> Dict[* -> *] is not "
        "supported.");
  }

  result_type operator()(const functions::IntegerType&) const {
    return std::make_unique<AppendImpl<value::IntegerValue>>();
  }

  result_type operator()(const functions::DoubleType&) const {
    return std::make_unique<AppendImpl<value::FloatValue>>();
  }

  result_type operator()(const functions::BoolType&) const {
    return std::make_unique<AppendImpl<value::BoolValue>>();
  }

  result_type operator()(const functions::StringType&) const {
    return std::make_unique<AppendImpl<value::StringValue>>();
  }

  result_type operator()(const functions::NullType&) const {
    return std::make_unique<AppendImpl<value::NullValue>>();
  }

  result_type operator()(const functions::AnyType&) const {
    throw std::invalid_argument(
        "Append: Dict[Any] -> String -> Any -> Dict[Any] is not "
        "supported. TODO: add support.");
  }

  result_type operator()(const functions::ListType& /*tau*/) const {
    return std::make_unique<AppendImpl<value::ArrayValue>>();
  }

  result_type operator()(const functions::DictType& /*tau*/) const {
    return std::make_unique<AppendImpl<value::ObjectValue>>();
  }

  result_type operator()(const functions::SchemaType& /*tau*/) const {
    return std::make_unique<AppendImpl<value::ObjectValue>>();
  }
};

}  // namespace

std::unique_ptr<IEvaluatable> MakeAppendImpl(const functions::Type& x_type) {
  return boost::apply_visitor(MakeAppendImplVisitor{}, x_type);
}

std::unique_ptr<IEvaluatable> Append::GetImplementation(
    const Type& specific_type) {
  auto mapping = utils::IsInstanceOf(specific_type, kType);
  if (!mapping) {
    throw UnsupportedType(
        std::format("Requested type does not match polymorphic type of Append: "
                    "{} !~ {} (requested !~ polymorphic)",
                    ToString(specific_type), ToString(kType)));
  }

  const functions::Type& x_type = mapping.value().at({'X'});
  return MakeAppendImpl(x_type);
}

}  // namespace magl::functions::library
