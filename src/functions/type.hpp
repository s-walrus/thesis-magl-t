#pragma once

#include <format>
#include <map>
#include <memory>
#include <typeindex>
#include <variant>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>

namespace magl::functions {

// Types

struct IntegerType {};
// TODO: rename to FloatType or rename all Float* to Double*
struct DoubleType {};
struct BoolType {};
struct StringType {};
struct NullType {};
struct AnyType {};
struct ListType;
struct DictType;
struct SchemaType;
struct FunctionType;
struct TypeVariable {
  uint64_t uid;
};

using Type =
    boost::variant<IntegerType, DoubleType, BoolType, StringType, NullType,
                   AnyType, boost::recursive_wrapper<ListType>,
                   boost::recursive_wrapper<DictType>,
                   boost::recursive_wrapper<SchemaType>,
                   boost::recursive_wrapper<FunctionType>, TypeVariable>;

struct ListType {
  Type value_type;
};

struct DictType {
  Type value_type;
};

struct SchemaType {
  // using map for deterministic order which is important for hashing
  std::map<std::string, Type> values;
};

struct FunctionType {
  Type argument;
  Type body;
};

// Equality

inline bool operator==(const IntegerType&, const IntegerType&) { return true; }

inline bool operator==(const DoubleType&, const DoubleType&) { return true; }

inline bool operator==(const BoolType&, const BoolType&) { return true; }

inline bool operator==(const StringType&, const StringType&) { return true; }

inline bool operator==(const NullType&, const NullType&) { return true; }

inline bool operator==(const AnyType&, const AnyType&) { return true; }

inline bool operator==(const ListType& lhs, const ListType& rhs) {
  return lhs.value_type == rhs.value_type;
}

inline bool operator==(const DictType& lhs, const DictType& rhs) {
  return lhs.value_type == rhs.value_type;
}

inline bool operator==(const SchemaType& lhs, const SchemaType& rhs) {
  return lhs.values == rhs.values;
}

inline bool operator==(const FunctionType& lhs, const FunctionType& rhs) {
  return lhs.argument == rhs.argument && lhs.body == rhs.body;
}

inline bool operator==(const TypeVariable& lhs, const TypeVariable& rhs) {
  return lhs.uid == rhs.uid;
}

// Hashing

inline size_t hash_value(const IntegerType t) {
  return boost::hash_value(typeid(t));
}

inline size_t hash_value(const DoubleType t) {
  return boost::hash_value(typeid(t));
}

inline size_t hash_value(const BoolType t) {
  return boost::hash_value(typeid(t));
}

inline size_t hash_value(const StringType t) {
  return boost::hash_value(typeid(t));
}

inline size_t hash_value(const NullType t) {
  return boost::hash_value(typeid(t));
}

inline size_t hash_value(const AnyType t) {
  return boost::hash_value(typeid(t));
}

inline size_t hash_value(const ListType t) {
  size_t seed = boost::hash_value(typeid(t));
  boost::hash_combine(seed, t.value_type);
  return seed;
}

inline size_t hash_value(const DictType t) {
  size_t seed = boost::hash_value(typeid(t));
  boost::hash_combine(seed, t.value_type);
  return seed;
}

inline size_t hash_value(const SchemaType t) {
  size_t seed = boost::hash_value(typeid(t));
  boost::hash_combine(seed, t.values.size());
  for (const auto& [key, value] : t.values) {
    boost::hash_combine(seed, key);
    boost::hash_combine(seed, value);
  }
  return seed;
}

inline size_t hash_value(const FunctionType t) {
  size_t seed = boost::hash_value(typeid(t));
  boost::hash_combine(seed, t.argument);
  boost::hash_combine(seed, t.body);
  return seed;
}

inline size_t hash_value(const TypeVariable t) {
  size_t seed = boost::hash_value(typeid(t));
  boost::hash_combine(seed, t.uid);
  return seed;
}

// Format

std::string ToString(const Type& t);

}  // namespace magl::functions

// Hashing

template <>
struct std::hash<magl::functions::IntegerType> {
  std::size_t operator()(const magl::functions::IntegerType& t) const noexcept {
    return magl::functions::hash_value(t);
  }
};

template <>
struct std::hash<magl::functions::DoubleType> {
  std::size_t operator()(const magl::functions::DoubleType& t) const noexcept {
    return magl::functions::hash_value(t);
  }
};

template <>
struct std::hash<magl::functions::BoolType> {
  std::size_t operator()(const magl::functions::BoolType& t) const noexcept {
    return magl::functions::hash_value(t);
  }
};

template <>
struct std::hash<magl::functions::StringType> {
  std::size_t operator()(const magl::functions::StringType& t) const noexcept {
    return magl::functions::hash_value(t);
  }
};

template <>
struct std::hash<magl::functions::NullType> {
  std::size_t operator()(const magl::functions::NullType& t) const noexcept {
    return std::hash<std::type_index>{}(std::type_index{typeid(t)});
  }
};

template <>
struct std::hash<magl::functions::AnyType> {
  std::size_t operator()(const magl::functions::AnyType& t) const noexcept {
    return magl::functions::hash_value(t);
  }
};

template <>
struct std::hash<magl::functions::ListType> {
  std::size_t operator()(const magl::functions::ListType& t) const noexcept {
    return magl::functions::hash_value(t);
  }
};

template <>
struct std::hash<magl::functions::DictType> {
  std::size_t operator()(const magl::functions::DictType& t) const noexcept {
    return magl::functions::hash_value(t);
  }
};

template <>
struct std::hash<magl::functions::SchemaType> {
  std::size_t operator()(const magl::functions::SchemaType& t) const noexcept {
    return magl::functions::hash_value(t);
  }
};

template <>
struct std::hash<magl::functions::FunctionType> {
  std::size_t operator()(const magl::functions::FunctionType& t) const
      noexcept {
    return magl::functions::hash_value(t);
  }
};

template <>
struct std::hash<magl::functions::TypeVariable> {
  std::size_t operator()(const magl::functions::TypeVariable& t) const
      noexcept {
    return magl::functions::hash_value(t);
  }
};

template <>
struct std::hash<magl::functions::Type> {
  std::size_t operator()(const magl::functions::Type& t) const noexcept {
    return boost::hash_value(t);
  }
};
