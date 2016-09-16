#include "x4.hpp"
#include "catch.hpp"

namespace x4 {

static constexpr auto N = 10;

template <std::size_t I, class=void>
struct recurse {
    auto operator()() const {
        if (true) return variant<int, decltype(recurse<I+1>()())>(0_c, 4);
        else return variant<int, decltype(recurse<I+1>()())>(1_c, recurse<I+1>()());
    }
};

template <std::size_t I>
struct recurse<I, void_if<(I >= N)>> {
    auto operator()() const {return 5;}
};


template <class T1, class T2>
class Alternative {
    T1 first; T2 second;

    template <class Window>
    struct checker<10, Window> {
        // --> here we need to delay type instantiation...
        variant<decltype(first(check_t<0>(), std::declval<Window &>())), decltype(second(check_t<0>(), std::declval<Window &>()))> value;
        checker(Window &w) : value() {}
    };

    template <std::size_t N, class Window>
    struct checker {
        variant<decltype(first(check_t<N+1>(), std::declval<Window &>())), decltype(second(check_t<N+1>(), std::declval<Window &>()))> value;
        checker(Window &w) : value() {}
    };


public:

    template <std::size_t N>
    auto operator()(check_t<N>, Window &w) const {
        return checker<N, Window>(w).value;
    }
};


TEST_CASE("recurse") {
    auto r = recurse<0>()();
    std::cout << r[0_c] << std::endl;
}

}
