#include "catch.hpp"
#include "x4.hpp"
#include <iostream>

namespace x4 {
using namespace literals;

TEST_CASE("0") {
    variant<false, char const *, double> ok;
    variant<false, char const *, double> const ok2(0_c, "okk");
    char const *x = ok2[0_c];
}

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
    REQUIRE(check(check_c, x, w));
    REQUIRE(check(check_c, x, w));
    REQUIRE(check(check_c, x, w));
    REQUIRE(!check(check_c, x, w));
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
    constexpr auto z = seq(x, y);
    auto result = parser(z)(s);
    dump(result[0_c], result[1_c]);
}

TEST_CASE("4") {
    optional_variant<false, int, double> var;
    REQUIRE(!bool(var));
    var.emplace(1_c, 5.5);
    REQUIRE(var[1_c] == 5.5);
    REQUIRE(bool(var));
}

struct test1 {test1(double) {}};
struct test2 {test2(bool) {}};

TEST_CASE("5") {
    variant<true, test1, test2> var1(0_c, 5.5);
    variant<true, test1, test2> var2{var1};
}

TEST_CASE("6") {
    constexpr auto x = 'a'_x;
    constexpr auto y = 'b'_x;
    constexpr auto z = x | y;

    std::string s = "abbc";
    auto w = make_window(s);
    auto c = check(check_c, z, w);
    REQUIRE(bool(check(check_c, z, w)));
    auto result = parse(parse_c, z, check(check_c, z, w));
    REQUIRE(result[1_c] == 'b');
    REQUIRE(!bool(check(check_c, z, w)));
}

TEST_CASE("7") {
    constexpr auto x = 'a'_x;
    constexpr auto y = 'b'_x;
    constexpr auto z = seq(x, y);

    std::string s = "abc";
    auto p = parser(z)(s);
    std::cout << p[0_c] << p[1_c] << std::endl;
}

TEST_CASE("8") {
    constexpr auto x = 'a'_x;
    constexpr auto y = 'b'_x;

    auto z = seq(x, y) % [](auto x, char y) {
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
    auto p = parser(seq("foo"_x, '_'_x, "bar"_x))(s);
    dump(p[0_c], p[1_c], p[2_c]);

    auto x = seq("foo"_x, '_'_x, "bar"_x) % [] (auto t, char u, auto v) {
        dump("x", t(), u, v());
    };
    parser(x)(s);

    auto y = seq("foo", '_'_x, "bar") % simplify % [] (auto t, auto u, auto v) {
        dump("y", t, u, v);
    };
    parser(y)(s);
}

TEST_CASE("10") {
    std::string s = "aaaaaaa";
    REQUIRE(parser(*'a'_x)(s) == s);
    REQUIRE(parser(+'a'_x)(s) == s);
    REQUIRE(parser(*'b'_x)(s) == "");
    std::cout << parser(+'b'_x).match(s) << std::endl;
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
    auto x = seq(size_x, size_x);
    auto p = parser(x, ' '_x)(s);
    dump(p[0_c], p[1_c]);
}

struct rule {};
constexpr auto implement(rule) {return seq(size_x, size_x);}

TEST_CASE("14") {
    std::string s = "98765     5674";
    auto p = parser(rule(), ' '_x)(s);
    dump(p[0_c], p[1_c]);
}

struct blah {
    std::vector<int> goo;
    optional_variant<true, blah> next;
};
//
TEST_CASE("15") {
    blah m_bah;
}

}
