#include <functions/library/map.hpp>

namespace magl::functions::library {

const Type Map::kType = FunctionType{
    FunctionType{TypeVariable{'X'}, TypeVariable{'Y'}},
    FunctionType{ListType{TypeVariable{'X'}}, ListType{TypeVariable{'Y'}}}};

}  // namespace magl::functions::library
