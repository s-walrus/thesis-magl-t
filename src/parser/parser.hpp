#pragma once

#include <parser/terms/semantic-graph.hpp>

#include <string>

namespace magl::parser {

terms::SemanticGraph Parse(std::string_view code);

}
