#include <functions/library/ducttape-var-map-at.hpp>

namespace magl::functions::library {

const Type VarMapAt::kType = FunctionType{
    DictType{TypeVariable{'X'}}, FunctionType{StringType{}, TypeVariable{'X'}}};

}  // namespace magl::functions::library
