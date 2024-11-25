#include <catch2/catch_test_macros.hpp>

#include <Canta/ResourceList.h>
#include <Canta/Buffer.h>


TEST_CASE("Resource reference counting", "[refcount]") {
    canta::ResourceList<canta::Buffer> list;
    auto handle = list.allocate();

    SECTION("copy") {
        auto tmp = handle;

        REQUIRE(tmp.count() == 2);
        REQUIRE(tmp.index() == handle.index());
    }

    SECTION("move") {
        auto tmp = std::move(handle);

        REQUIRE(tmp.count() == 1);
        REQUIRE(handle.index() == -1);
    }

    SECTION("destroy") {
        handle = {};

        REQUIRE(handle.count() == 0);
    }

}