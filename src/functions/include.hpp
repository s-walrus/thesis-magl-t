#pragma once

#include <unordered_map>

#include <functions/function-factory.hpp>

namespace magl::functions {

using FunctionsLibrary =
    std::unordered_map<std::string /*syntax_name*/,
                       std::unique_ptr<PolymorphicFunctionFactory>>;

FunctionsLibrary MakeDefaultLibrary();

}  // namespace magl::functions
