#pragma once

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <boost/variant.hpp>
#include <boost/variant/get.hpp>

#include <functions/evaluatable.hpp>

namespace magl::value {

// Values must be trivially relocatable as in
// https://open-std.org/JTC1/SC22/WG21/docs/papers/2020/p1144r5.html
// Most types from STL are not guaranteed to be trivially relocatable, but this
// implementation assumes that std::string, std::unordered_map and std::vector
// are trivially relocatable, which may lead to UB on some implementations of
// STL. That is an issue.

using IntegerValue = int64_t;
using FloatValue = double;
using BoolValue = bool;
using StringValue = std::string;
struct NullValue {};
// Non-owning pointer to an evaluatable
using LambdaValue = functions::IEvaluatable*;

using Value = boost::make_recursive_variant<
    IntegerValue, FloatValue, BoolValue, StringValue, NullValue, LambdaValue,
    std::vector<boost::recursive_variant_>,
    std::unordered_map<std::string, boost::recursive_variant_> >::type;

using ObjectValue = std::unordered_map<std::string, Value>;
// TODO: Consider making value types for array of Integer, Float, etc. directly
// TODO: Same for Object
using ArrayValue = std::vector<Value>;

// Helpers

/// Returns whether the value is representable as a JSON entity
bool IsPrimitive(const Value& value);

// Comparators

inline bool operator==(const NullValue&, const NullValue&) { return true; }

}  // namespace magl::value
