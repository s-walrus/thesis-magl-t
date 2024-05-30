#pragma once

#include <array>
#include <cstddef>

namespace magl::functions {

// TODO: Add a memory-safe interface for ArgsContainer and ValueHolder:
// initialize a wrapper for ArgsContainer with known-ahead types with zero
// overhead that guards ownership and provides type-safe access

// TODO: Move somewhere
const size_t kMaxArgs = 32;
const size_t kMaxValueSize = 64;

using ValueHolder = std::array<std::byte, kMaxValueSize>;
using ArgsContainer = std::array<ValueHolder, kMaxArgs>;

struct IEvaluatable {
  virtual ~IEvaluatable() = default;
  virtual void Evaluate(ArgsContainer* args, ValueHolder* to) = 0;
};

}  // namespace magl::functions
