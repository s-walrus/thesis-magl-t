#include <functions/utils/type-equivalent.hpp>

namespace magl::functions::utils {

namespace {

class IsInstanceOfVisitor : public boost::static_visitor<bool> {
 public:
  IsInstanceOfVisitor() = default;

  result_type operator()(const Type& subtype,
                         const TypeVariable& supertype) const {
    auto find_mapping = mapping_.find(supertype);
    if (find_mapping == mapping_.end()) {
      mapping_.emplace_hint(find_mapping, supertype, subtype);
      return true;
    }

    return subtype == find_mapping->second;
  }

  result_type operator()(const Type& subtype,
                         const FunctionType& supertype) const {
    if (!boost::get<FunctionType>(&subtype)) {
      return false;
    }

    const FunctionType subtype_value = boost::get<FunctionType>(subtype);
    if (!boost::apply_visitor(*this, subtype_value.argument,
                              supertype.argument)) {
      return false;
    }
    if (!boost::apply_visitor(*this, subtype_value.body, supertype.body)) {
      return false;
    }

    return true;
  }

  result_type operator()(const Type& subtype, const ListType& supertype) const {
    if (!boost::get<ListType>(&subtype)) {
      return false;
    }

    return boost::apply_visitor(*this, boost::get<ListType>(subtype).value_type,
                                supertype.value_type);
  }

  result_type operator()(const Type& subtype, const DictType& supertype) const {
    if (!boost::get<DictType>(&subtype)) {
      return false;
    }

    return boost::apply_visitor(*this, boost::get<DictType>(subtype).value_type,
                                supertype.value_type);
  }

  result_type operator()(const Type& subtype,
                         const SchemaType& supertype) const {
    if (!boost::get<SchemaType>(&subtype)) {
      return false;
    }

    const SchemaType subtype_value = boost::get<SchemaType>(subtype);
    if (subtype_value.values.size() != supertype.values.size()) {
      return false;
    }
    for (const auto& [key, value] : subtype_value.values) {
      const auto find_value = supertype.values.find(key);
      if (find_value == supertype.values.end()) {
        return false;
      }
      if (!boost::apply_visitor(*this, value, find_value->second)) {
        return false;
      }
    }

    return true;
  }

  result_type operator()(const Type& subtype, const IntegerType&) const {
    return boost::get<IntegerType>(&subtype);
  }

  result_type operator()(const Type& subtype, const DoubleType&) const {
    return boost::get<DoubleType>(&subtype);
  }

  result_type operator()(const Type& subtype, const StringType&) const {
    return boost::get<StringType>(&subtype);
  }

  result_type operator()(const Type& subtype, const BoolType&) const {
    return boost::get<BoolType>(&subtype);
  }

  result_type operator()(const Type& subtype, const NullType&) const {
    return boost::get<NullType>(&subtype);
  }

  result_type operator()(const Type& subtype, const AnyType&) const {
    return boost::get<AnyType>(&subtype);
  }

  std::unordered_map<TypeVariable, Type> GetVariableMapping() const {
    return mapping_;
  }

 private:
  // Using mutable is a workaround for apply_visitor which only accepts const
  // visitor
  mutable std::unordered_map<TypeVariable, Type> mapping_;
};

}  // namespace

std::optional<std::unordered_map<TypeVariable, Type>> IsInstanceOf(
    const Type& subtype, const Type& supertype) {
  const IsInstanceOfVisitor v;
  const bool is_instance = boost::apply_visitor(v, subtype, supertype);
  if (!is_instance) {
    return std::nullopt;
  }
  return v.GetVariableMapping();
}

}  // namespace magl::functions::utils
