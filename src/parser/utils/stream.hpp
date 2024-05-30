#include <optional>

namespace magl::parser::utils {

template <typename Stream, typename Value>
class PeekableStream {
 public:
  explicit PeekableStream(Stream* in) : in_(in) {}

  Value Next() {
    if (buf_.has_value()) {
      Value result = std::move(buf_.value());
      buf_.reset();
      return result;
    }

    Value result;
    *in_ >> result;
    return result;
  }

  Value const* Peek() {
    if (!buf_.has_value()) {
      buf_.emplace(Next());
    }
    return &buf_.value();
  }

 private:
  Stream* in_;
  std::optional<Value> buf_;
};

}  // namespace magl::parser::utils
