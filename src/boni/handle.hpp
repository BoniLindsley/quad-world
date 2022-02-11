#pragma once
#include <memory>

namespace boni {

template <typename HandleType, void (*DeleteFunction)(HandleType)>
class deleter {
public:
  // Custom deleter for `unique_ptr`.
  // Empty base class optimisation allows this to be done
  // without memory allocation overhead.
  using pointer = HandleType;
  void operator()(HandleType element) const
      noexcept(noexcept(DeleteFunction(element))) {
    DeleteFunction(element);
  }
};

template <
    typename HandleType, void (*DeleteFunction)(HandleType),
    typename Deleter = deleter<HandleType, DeleteFunction>>
class handle : public std::unique_ptr<HandleType, Deleter> {
public:
  // Use `unique_ptr` to handle lifetime,
  // and to delete copy constructor and assignment.
  using parent_type = std::unique_ptr<HandleType, Deleter>;
  // This is usually `HandleType*`,
  // but the custom deleter forces it to become `HandleType`.
  using handle_type = typename parent_type::pointer;

  handle() = default; // Zero initialise pointer.
  // This constructor starts managing the given handle pointer.
  // Do not allow implicit conversion from a pointer.
  // since this class "deletes" the pointer when leaving its scope.
  // If the given pointer is non-zero, assume it is valid.
  explicit handle(handle_type valid_pointer) noexcept
      : parent_type(valid_pointer) {}

  // But allow implicit conversion back to a pointer for use with C API.
  operator handle_type() { return parent_type::get(); }
};

template <void (*DeleteFunction)()> class cleanup {
public:
  ~cleanup() { DeleteFunction(); }
};

} // end namespace boni