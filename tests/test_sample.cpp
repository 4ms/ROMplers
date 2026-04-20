#include <doctest/doctest.h>

TEST_CASE("framework verification: intentionally failing test") {
    CHECK(1 == 2);
}
