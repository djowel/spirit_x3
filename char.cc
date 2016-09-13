#include "catch.hpp"
#include "x4/parse/declare.hpp"
#include "x4/directive/lexeme.hpp"
#include "x4/string/char_set.hpp"
#include "x4/string/char_predicate.hpp"
#include "x4/numeric/int.hpp"
#include "x4/operator/list.hpp"
#include "x4/operator/restriction.hpp"
#include "x4/operator/optional.hpp"
#include "x4/operator/kleene.hpp"
#include "x4/operator/plus.hpp"
#include "x4/operator/alternative.hpp"
#include "x4/parse/parse.hpp"
#include "x4/window/window.hpp"
#include "x4/string/literal_char.hpp"
#include "x4/string/literal_string.hpp"
#include "x4/operator/sequence.hpp"
#include "x4/operator/transform.hpp"
#include "x4/parse/variant.hpp"
#include "x4/string/window_predicate.hpp"
#include <iostream>

namespace x4 {

template <class T, class ...Ts>
void dump(T const &t, Ts const &...ts) {
    std::cout << std::boolalpha << t;
    int _[] = {(std::cout << ", " << ts, 0)...};
    std::cout << std::endl;
}

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
    dump(parser(x)(s));
    dump(parser(x)(s));
    dump(parser(x)(s));
    dump(parser(x)(s));
}

TEST_CASE("3") {
    std::string s = "aaa";
    constexpr auto x = 'a'_x;
    constexpr auto y = 'a'_x;
    constexpr auto z = x >> y;
    auto result = parser(z)(s);
    dump(result[0_c], result[1_c]);
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
    REQUIRE(result[1_c] == 'b');
    REQUIRE(!bool(check_of(z, w)));
}

TEST_CASE("7") {
    constexpr auto x = 'a'_x;
    constexpr auto y = 'b'_x;
    constexpr auto z = x >> y;

    std::string s = "abc";
    auto w = make_window(s);
    dump(success_of(z, check_of(z, w)));
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
    REQUIRE(parser(z)(s) == 5.5);
}

TEST_CASE("9") {
    std::string s = "foo_bar";
    dump(parser("foo_bar"_x)(s));
    auto p = parser("foo"_x >> '_'_x >> "bar"_x)(s);
    dump(p[0_c], p[1_c], p[2_c]);

    auto x = ("foo"_x >> '_'_x >> "bar"_x) % [] (auto t, char u, auto v) {
        dump(t(), u, v());
    };

    parser(x, s);
}

TEST_CASE("10") {
    std::string s = "aaaaaaa";
    REQUIRE(parser(*'a'_x)(s) == s);
    REQUIRE(parser(+'a'_x)(s) == s);
    REQUIRE(parser(*'b'_x)(s) == "");
    REQUIRE_THROWS(parser(+'b'_x)(s));
    REQUIRE(parser(*('a'_x % [](auto s){}))(s).size() == 7);
}

TEST_CASE("11") {
    std::string s = "opional";
    REQUIRE(!parser(~"optional"_x)(s));
    REQUIRE(parser(~"opional"_x)(s));
    REQUIRE(parser("opional"_x | "optional"_x)(s)[0_c] == "opional");
}

TEST_CASE("12") {
    std::string s = "1234";
    auto p = parser(size_x)(s);
    dump(p);
    std::string s2 = "-1234";
    auto p2 = parser(int_x)(s2);
    dump(p2);
}

TEST_CASE("13") {
    std::string s = "98765     5674";
    auto x = size_x >> size_x;
    auto p = parser(x, ' '_x)(s);
    dump(p[0_c], p[1_c]);
}

struct rule {};
constexpr auto implement(rule) {return size_x >> size_x;}

TEST_CASE("14") {
    std::string s = "98765     5674";
    auto p = parser(rule(), ' '_x)(s);
    dump(p[0_c], p[1_c]);
}


// main simple things missing:
// NUMBERS
// SKIPPING
// that's it I guess.

}
