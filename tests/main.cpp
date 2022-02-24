// Internal headers.
#include <boni/handle.hpp>

// External dependencies.
#define CATCH_CONFIG_MAIN
#include <catch.hpp>

int value{};
void set_value(int new_value) { value = new_value; }
void reset_value() { value = 0; }

TEST_CASE("deleter is callable") {
  value = 0;
  using Deleter = boni::deleter<int, set_value>;
  Deleter deleter{};
  deleter(1);
  REQUIRE(value == 1);
}

TEST_CASE("handle stores given value") {
  using Handle = boni::handle<int, set_value>;
  Handle handle{1};
  REQUIRE(handle.get() == 1);
}

TEST_CASE("handle default initialises to null") {
  using Handle = boni::handle<int, set_value>;
  Handle handle;
  REQUIRE(handle.get() == 0);
}

TEST_CASE("handle zero initialises to null") {
  using Handle = boni::handle<int, set_value>;
  Handle handle{};
  REQUIRE(handle.get() == 0);
}

TEST_CASE("handle casts to handle type") {
  using Handle = boni::handle<int, set_value>;
  Handle handle{1};
  int stored_value = handle;
  REQUIRE(stored_value == 1);
}

TEST_CASE("handle calls deleter with stored value") {
  value = 0;
  using Handle = boni::handle<int, set_value>;
  { Handle handle{1}; }
  REQUIRE(value == 1);
}

TEST_CASE("handle does not call deleter if null") {
  value = 1;
  using Handle = boni::handle<int, set_value>;
  { Handle handle; }
  REQUIRE(value == 1);
}

TEST_CASE("cleanup call deleter function on scope exit") {
  value = 1;
  using Cleanup = boni::cleanup<reset_value>;
  { Cleanup cleanup; }
  REQUIRE(value == 0);
}
