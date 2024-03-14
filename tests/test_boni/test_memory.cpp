// Corresponding headers.
#include <boni/memory.hpp>

// Internal headers
#include <boni/type_traits.hpp>

// External libraries.
#include <catch.hpp>
#include <catch2/catch_test_macros.hpp>

// Test conditions for use in `std::unique_ptr`.

TEST_CASE("nullable<int> is NullablePointer") {
  using nullable_int = boni::memory::nullable<int>;
  REQUIRE(boni::type_traits::is_nullable_pointer<nullable_int>::value);
}
TEST_CASE("nullable<int> casts to false if default") {
  REQUIRE(not boni::memory::nullable<int>());
}

TEST_CASE("nullable<int> casts to false if nullptr") {
  REQUIRE(not boni::memory::nullable<int>(nullptr));
}

TEST_CASE("nullable<int> casts to false if zero") {
  REQUIRE(not boni::memory::nullable<int>(0));
}

TEST_CASE("nullable<int> casts to true if non-zero") {
  REQUIRE(boni::memory::nullable<int>(1));
}

TEST_CASE("nullable<int> casts to true if negative") {
  REQUIRE(boni::memory::nullable<int>(-1));
}

TEST_CASE("nullable<int> compares false to nullptr if non-zero") {
  REQUIRE(boni::memory::nullable<int>(1) != nullptr);
}

TEST_CASE("nullable<int> compares true to nullptr if default") {
  REQUIRE(boni::memory::nullable<int>() == nullptr);
}

// Test static_deleter

int value{};
void set_value(int new_value) { value = new_value; }
void reset_value() { value = 0; }

TEST_CASE("static_deleter is callable") {
  value = 0;
  boni::memory::static_deleter<int, void, set_value> deleter;
  deleter(1);
  REQUIRE(value == 1);
}

// Test handle value and deleter being called

TEST_CASE("handle stores given value") {
  boni::memory::handle<int, void, set_value> handle{1};
  REQUIRE(handle.get() == 1);
}

TEST_CASE("handle default initialises to null") {
  boni::memory::handle<int, void, set_value> handle;
  REQUIRE(handle.get() == 0);
}

TEST_CASE("handle zero initialises to null") {
  boni::memory::handle<int, void, set_value> handle{};
  REQUIRE(handle.get() == 0);
}

TEST_CASE("handle casts to handle type") {
  boni::memory::handle<int, void, set_value> handle{1};
  int stored_value = handle;
  REQUIRE(stored_value == 1);
}

TEST_CASE("handle calls deleter with stored value") {
  value = 0;
  { boni::memory::handle<int, void, set_value> handle{1}; }
  REQUIRE(value == 1);
}

TEST_CASE("handle does not call deleter if null") {
  value = 1;
  { boni::memory::handle<int, void, set_value> handle; }
  REQUIRE(value == 1);
}
