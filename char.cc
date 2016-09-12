#include "catch.hpp"
#include "x4/operator/alternative.hpp"
#include "x4/parse/parse.hpp"
#include "x4/window/window.hpp"
#include "x4/char/literal.hpp"
#include "x4/operator/sequence.hpp"
#include "x4/operator/transform.hpp"
#include "x4/parse/variant.hpp"
#include <iostream>

namespace x4 {

TEST_CASE("1") {
    std::string s = "aaa";
    auto w = make_window(s);
    constexpr auto x = 'a'_x;
    REQUIRE(x.check(w));
    REQUIRE(x.check(w));
    REQUIRE(x.check(w));
    REQUIRE(!x.check(w));
}

TEST_CASE("2") {
    std::string s = "aaa";
    constexpr auto x = 'a'_x;
    std::cout << parse(s, x) << std::endl;
    std::cout << parse(s, x) << std::endl;
    std::cout << parse(s, x) << std::endl;
    std::cout << parse(s, x) << std::endl;
}

TEST_CASE("3") {
    std::string s = "aaa";
    constexpr auto x = 'a'_x;
    constexpr auto y = 'a'_x;
    constexpr auto z = x >> y;
    auto result = parse(s, z);
    std::cout << result[0_c] << result[1_c] << std::endl;
}


TEST_CASE("4") {
    optional_variant<int, double> var;
    REQUIRE(!bool(var));
    var.emplace(1_c, 5.5);
    REQUIRE(var[1_c] == 5.5);
    REQUIRE(bool(var));
}

struct test1 {test1(double) {}};
struct test2 {test2(bool) {}};

TEST_CASE("5") {
    variant<test1, test2> var1(0_c, 5.5);
    variant<test1, test2> var2{var1};
}

TEST_CASE("6") {
    constexpr auto x = 'a'_x;
    constexpr auto y = 'b'_x;
    constexpr auto z = x | y;

    std::string s = "abbc";
    auto w = make_window(s);
    REQUIRE(bool(check_of(z, w)));
    REQUIRE(bool(check_of(z, w)));

    auto result = parse_of(z, check_of(z, w));
    REQUIRE(result == 'b');
    REQUIRE(!bool(check_of(z, w)));
}

TEST_CASE("7") {
    constexpr auto x = 'a'_x;
    constexpr auto y = 'b'_x;
    constexpr auto z = x >> y;

    std::string s = "abc";
    auto w = make_window(s);
    std::cout << success_of(z, check_of(z, w)) << std::endl;
}


TEST_CASE("8") {
    constexpr auto x = 'a'_x;
    constexpr auto y = 'b'_x;

    auto z = (x >> y) % [](auto x, char y) {
        REQUIRE(x() == 'a');
        REQUIRE(y == 'b');
        return 5.5;
    };

    std::string s = "abc";
    REQUIRE(parse(s, z) == 5.5);
    //std::cout << success_of(z, check_of(z, w)) << std::endl;
}

}
