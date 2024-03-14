#pragma once

// Internal headers.
#include "./type_traits.hpp"

// Standard library.
#include <cstddef>
#include <memory>
#include <type_traits>

/** \brief Contains convenience RAII wrappers.  */
namespace boni::memory {

/** \brief Wraps a type so that it satisfies `NullablePointer`.
 *
 *  \tparam handle_t
 *          The type that is to be used as a "pointer".
 *          Effectively, it is the type of the data to be stored
 *          in instances of this class.
 *  \tparam null_value
 *          The value to be stored
 *          when the "pointer" represents a "null" pointer.
 *
 *  The type `T` given to `std::unique_ptr<T, Deleter>`
 *  must be a `NullablePointer`.
 *  However, commonly used handle types such as `int`
 *  do not satisfy this requirement.
 *  This class wraps such types so that they do.
 */
template <typename handle_t, handle_t null_value = handle_t{}>
class nullable {
public:
  /** \brief Refers to this type. */
  using self_type = nullable;

  /** \brief Refers to stored type. */
  using value_type = handle_t;

  /** \brief Initialises stored value to `null_value`.
   *
   *  A `NullablePointer` must be `DefaultConstructible`
   *  and value initialises to its null value.
   */
  nullable() = default;

  /** \brief Initialises stored value to `null_value`.
   *
   * A `NullablePointer` must convert a `nullptr` to its null value.
   */
  nullable(std::nullptr_t) : nullable{} {};

  /** \brief Converts from the underlying type. For convenience.
   *
   *  This is needed so that `std::unique_ptr<nullable<T>>`
   *  can accept instances of type `T` in its constructor.
   */
  nullable(value_type new_value) : value{new_value} {}

  /** \brief Returns whether `null_value` is stored.
   *
   *  A `NullablePointer` must return `false`
   *  if and only if its value is equivalent to its null value.
   */
  explicit operator bool() const { return value != null_value; }

  /** \brief Converts to underlying type. For convenience. */
  operator value_type() const { return value; }

  /** \brief Compares the stored `value_type` data.
   *
   *  A `NullablePointer` must be `EqualityComparable`
   *  and must also be comparable with `nullptr` using `==`.
   */
  friend auto operator==(self_type left, self_type right) -> bool {
    return static_cast<value_type>(left) ==
           static_cast<value_type>(right);
  }

  /** \brief Compares the stored `value_type` data.
   *
   *  A `NullablePointer` must be comparable with `nullptr` using `!=`.
   */
  friend auto operator!=(self_type left, self_type right) -> bool {
    return !(left == right);
  }

  /** \brief The underlying stored data. */
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  value_type value{null_value};
};

/** \brief Callable wrapping delete function of a handle.
 *
 *  \tparam handle_t
 *          The handle type representing a resource.
 *  \tparam return_type
 *          The type returned by `destroy_function`.
 *          Only for defining the `destroy_function` template argument.
 *  \tparam destroy_function
 *          A pointer to a function that releases resource
 *          represented by instances of `handle_t`.
 *          It need not handle the case with input `handle_type()`
 *          which will be filtered out by this class.
 *
 *  This class wraps a function pointer
 *  usable as a `std::unique_ptr` deleter.
 *  This avoids potential memory overhead
 *  from passing a function pointer directly as a deleter.
 *
 *  ```cpp
 *  #include "./boni/memory.hpp"
 *  #include <cstdio>
 *
 *  using file = boni::memory::handle<FILE*, int, std::fclose>;
 *  ```
 *
 *  Note that this class will likely become unnecessary
 *  when lambdas can be used directly as deleter.
 */
template <
    typename handle_t, typename return_type,
    return_type (*destroy_function)(handle_t),
    typename pointer_t = handle_t>
class static_deleter {
public:
  /** \brief The `handle_type` that the `destroy_function` acts on.
   *
   *  In particular, this is the type that C API acts on.
   */
  using handle_type = handle_t;

  /** \brief The type to be stored by `std::unique_ptr`.
   *
   *  This is for use as `Deleter::pointer`
   *  in the `std::unique_ptr<T, Deleter>` interface.
   *  It must be convertible to `handle_type`.
   */
  using pointer = pointer_t;

  /** \brief Destroy the given handle.
   *
   *  That is, this callable simply forwards to `destroy_function`,
   *  but only if the argument is not "null".
   */
  void operator()(pointer handle_to_delete) {
    if (handle_to_delete == pointer()) {
      return;
    }
    destroy_function(handle_to_delete);
  }
};

/** \brief Wrapper around `std::unique_ptr` for use with C API.
 *
 *  \tparam handle_t
 *          The handle type representing a resource.
 *  \tparam return_type
 *          The type returned by `destroy_function`.
 *          Only for defining the `destroy_function` template argument.
 *  \tparam destroy_function
 *          A pointer to a function that releases resource
 *          represented by instances of `handle_t`.
 *
 *  This class is for managing resource lifetime.
 *  The "acquisition" part of RAII is assumed to be done by the caller,
 *  with the resulting handle being given immediately to this class,
 *  with no throwing code in between.
 *
 *  The class interface is essentially `std::unique_ptr`,
 *  except there is implicit conversion to the `handle_t`.
 *  This is to ease usage with C API
 *  which frequently accepts the underlying handle,
 *  by removing the need to call `get()`.
 *
 *  The following is an example of how the class is intended to be used:
 *
 *  ```cpp
 *  // Internal headers.
 *  #include "./boni/memory.hpp"
 *
 *  // Standard libraries.
 *  #include <cstdio>
 *  #include <memory>
 *  #include <string>
 *
 *  using file = boni::memory::handle<FILE*, int, std::fclose>;
 *
 *  auto main(int, char**) -> int {
 *    // Pass a created handle to the class immediately.
 *    auto filename = "hello.txt";
 *    file hello_file{std::fopen(filename, "w+b")};
 *    if (not hello_file) {
 *      std::fprintf(stderr, "Unable to open file: %s\n", filename);
 *      return 1;
 *    }
 *
 *    // Do things with the handle.
 *    std::string buffer = "Hello, world!\n";
 *    // There is implicit conversion to the underlying handle type.
 *    auto result = std::fwrite(
 *        buffer.c_str(), sizeof(decltype(buffer)::value_type),
 *        buffer.size(), hello_file);
 *    if (result < buffer.size()) {
 *      std::fprintf(stderr, "Unable to write to file: %s\n", filename);
 *      return 1;
 *    }
 *
 *    // Automatically call `fclose(hello_file.get());`.
 *    // Return value is discarded.
 *  }
 *  ```
 */
template <
    typename handle_t, typename return_t,
    return_t(destroy_function)(handle_t),
    typename parent_t = std::unique_ptr<
        handle_t,
        static_deleter<
            handle_t, return_t, destroy_function,
            typename std::conditional<
                type_traits::is_nullable_pointer<handle_t>::value,
                handle_t, nullable<handle_t>>::type>>>
class handle : public parent_t {
public:
  using parent_t::parent_t;

  /** \brief The handle type for use with C APIs. */
  using handle_type = handle_t;

  /** \brief Implicit conversion to `handle_type`.
   *
   *  This is to ease usage with C API
   *  which frequently accepts the underlying handle.
   *
   *  Note that this conversion may not work if the function is a macro,
   *  since the compiler may not be able to deduce
   *  what the type `handle` should convert to.
   *  This is common, for example, in ncurses API.
   *  Fallback to `get()` in such cases.
   */
  operator handle_type() const { return get(); }

  /** \brief Returns the C API resource handle.
   *
   *  This may be different from `parent_type::pointer`
   *  due to nullable wrapper.
   */
  auto get() const -> handle_type { return parent_t::get(); }
};

} // namespace boni::memory