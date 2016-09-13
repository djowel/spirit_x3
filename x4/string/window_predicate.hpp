#pragma once
#include "../parse/common.hpp"

namespace x4 {

/******************************************************************************************/

template <class Predicate>
class window_predicate : Predicate, expression_base {
    Predicate predicate;

public:

    template <class ...Ts>
    constexpr window_predicate(Ts &&...ts) : predicate(std::forward<Ts>(ts)...) {}

    template <class Window>
    auto check(Window &w) const {return predicate(w);}

    template <class T> constexpr auto parse(T const &t) const {return t;}
};

/******************************************************************************************/

struct eoi_t : window_predicate<std::decay_t<decltype(hana::to<bool>)>> {constexpr eoi_t() {}};
static constexpr auto eoi = eoi_t();

/******************************************************************************************/

}
