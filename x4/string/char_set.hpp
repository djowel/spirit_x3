#pragma once
#include "../parse/common.hpp"
#include <boost/hana/none_of.hpp>
#include <boost/hana/equal.hpp>

namespace x4 {

template <class ...Chars>
class char_set : parser_base {
    hana::tuple<Chars...> set;

public:

    template <class ...Ts>
    explicit constexpr char_set(Ts &&...ts) : set(std::forward<Ts>(ts)...) {}

    template <class Window>
    auto operator()(check_base, Window &w) const {
        if (hana::any_of(set, hana::equal.to(*w))) return *(w++);
        else return static_cast<std::decay_t<decltype(*w)>>(0);
    }

    template <class T>
    constexpr auto operator()(parse_base, T t) const {return t;}
};

struct eol_t : char_set<hana::char_<'\n'>, hana::char_<'\r'>> {constexpr eol_t() {}};
static constexpr auto eol = eol_t();

}
