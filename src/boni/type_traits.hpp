#pragma once

// Standard library.
#include <cstddef>
#include <type_traits>

namespace boni::type_traits {

namespace details {

// Reference: https://stackoverflow.com/a/50631844
template <typename T, typename U> struct is_equal_checker {
  template <typename V = T, typename W = U>
  static auto is_equal(V&& v, W&& w)
      -> decltype(v == w, void(), std::true_type{});
  static auto is_equal(...) -> std::false_type;
  using result =
      decltype(is_equal(std::declval<T>(), std::declval<U>()));
  static constexpr bool value = result::value;
};

// Reference: https://stackoverflow.com/a/50631844
template <typename T, typename U> struct is_not_equal_checker {
  template <typename V = T, typename W = U>
  static auto is_not_equal(V&& v, W&& w)
      -> decltype(v == w, void(), std::true_type{});
  static auto is_not_equal(...) -> std::false_type;
  using result =
      decltype(is_not_equal(std::declval<T>(), std::declval<U>()));
  static constexpr bool value = result::value;
};

} // namespace details

template <typename T, typename U>
struct is_equality_comparable_with
    : details::is_equal_checker<T, U>::result {};

template <typename T>
struct is_equality_comparable : details::is_equal_checker<T, T>::result {
};

template <typename T>
struct is_nullable_pointer
    : std::integral_constant<
          bool,
          is_equality_comparable<T>::value and
              std::is_default_constructible<T>::value and
              std::is_copy_constructible<T>::value and
              std::is_copy_assignable<T>::value and
              std::is_swappable<T>::value and
              std::is_destructible<T>::value and
              std::is_constructible<T, std::nullptr_t>::value and
              std::is_assignable<T&, std::nullptr_t>::value and
              details::is_not_equal_checker<T, T>::value and
              is_equality_comparable_with<T, std::nullptr_t>::value and
              is_equality_comparable_with<std::nullptr_t, T>::value and
              details::is_not_equal_checker<T, std::nullptr_t>::value and
              details::is_not_equal_checker<std::nullptr_t, T>::value and
              true> {};

} // namespace boni::type_traits
