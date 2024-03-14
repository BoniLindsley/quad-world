// Corresponding headers.
#include <boni/type_traits.hpp>

// External libraries.
#include <catch.hpp>

TEST_CASE("Integers satisfy EqualityComparableWith Integers") {
  REQUIRE(
      boni::type_traits::is_equality_comparable_with<char, int>::value);
  REQUIRE(
      boni::type_traits::is_equality_comparable_with<int, bool>::value);
  REQUIRE(boni::type_traits::is_equality_comparable_with<
          unsigned, long>::value);
}

TEST_CASE("Integers not satisfy EqualityComparableWith with nullptr") {
  REQUIRE(not boni::type_traits::is_equality_comparable_with<
          int, std::nullptr_t>::value);
}

TEST_CASE(
    "Integers not satisfy EqualityComparableWith with Empty class") {
  class Empty {};
  REQUIRE(not boni::type_traits::is_equality_comparable_with<
          int, Empty>::value);
}

TEST_CASE("Integers satisfy EqualityComparable") {
  REQUIRE(boni::type_traits::is_equality_comparable<char>::value);
  REQUIRE(boni::type_traits::is_equality_comparable<int>::value);
  REQUIRE(boni::type_traits::is_equality_comparable<unsigned>::value);
}

TEST_CASE("Empty class does not satisfy EqualityComparable") {
  class Empty {};
  REQUIRE(not boni::type_traits::is_equality_comparable<Empty>::value);
}

TEST_CASE("Pointers satisfy NullablePointer") {
  REQUIRE(boni::type_traits::is_nullable_pointer<char*>::value);
  REQUIRE(boni::type_traits::is_nullable_pointer<int*>::value);
  REQUIRE(boni::type_traits::is_nullable_pointer<void*>::value);
  REQUIRE(boni::type_traits::is_nullable_pointer<void**>::value);
  REQUIRE(boni::type_traits::is_nullable_pointer<std::nullptr_t>::value);
}

TEST_CASE("Integers do not satisfy NullablePointer") {
  REQUIRE(not boni::type_traits::is_nullable_pointer<char>::value);
  REQUIRE(not boni::type_traits::is_nullable_pointer<int>::value);
  REQUIRE(not boni::type_traits::is_nullable_pointer<unsigned>::value);
}
