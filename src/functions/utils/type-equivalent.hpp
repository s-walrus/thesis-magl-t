#pragma once

#include <functions/type.hpp>

namespace magl::functions::utils {

/// Returns whether supertype can be converted to subtype with variable
/// substitutions
std::optional<std::unordered_map<TypeVariable, Type>> IsInstanceOf(
    const Type& subtype, const Type& supertype);

}  // namespace magl::functions::utils
