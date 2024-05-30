#include <functions/include.hpp>

#include <functions/library/add.hpp>
#include <functions/library/append.hpp>
#include <functions/library/debug-get-any-one.hpp>
#include <functions/library/ducttape-get-var.hpp>
#include <functions/library/ducttape-var-map-at.hpp>
#include <functions/library/insert.hpp>
#include <functions/library/map.hpp>
#include <functions/library/multiply.hpp>

namespace magl::functions {

FunctionsLibrary MakeDefaultLibrary() {
  FunctionsLibrary result;

  result.emplace("Add", std::make_unique<library::Add>());
  result.emplace("Multiply", std::make_unique<library::Multiply>());
  result.emplace("Insert", std::make_unique<library::Insert>());
  result.emplace("Append", std::make_unique<library::Append>());
  result.emplace("GetAnyOne", std::make_unique<library::GetAnyOne>());
  result.emplace("GetVar", std::make_unique<library::GetVar>());
  result.emplace("VarMapAt", std::make_unique<library::VarMapAt>());
  result.emplace("Map", std::make_unique<library::Map>());

  return result;
}

}  // namespace magl::functions
