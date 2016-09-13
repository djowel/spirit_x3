#pragma once
#include "parse.hpp"

namespace x4 {

/******************************************************************************************/

void implement(...);

template <class P, class=void> struct has_adl_impl : std::false_type {};
template <class P> struct has_adl_impl<P, void_if<!(std::is_same<decltype(implement(std::declval<P const>())), void>::value)>> : std::true_type {};

template <class P, class=void> struct has_member_impl : std::false_type {};
template <class P> struct has_member_impl<P, void_if<!(std::is_same<decltype(std::declval<P const>().implement()), void>::value)>> : std::true_type {};

/******************************************************************************************/

template <class P>
struct implementation<P, void_if<has_adl_impl<P>::value && !has_member_impl<P>::value>> {
    template <class Window>
    constexpr auto check(P const &p, Window &w) const {return check_of(implement(p), w);}

    template <class Data>
    constexpr auto success(P const &p, Data const &data) const {return success_of(implement(p), data);}

    template <class ...Args>
    constexpr auto parse(P const &p, Args &&...args) const {return parse_of(implement(p), std::forward<Args>(args)...);}
};

/******************************************************************************************/

template <class P>
struct implementation<P, void_if<has_member_impl<P>::value>> {
    template <class Window>
    constexpr auto check(P const &p, Window &w) const {return check_of(p.implement(), w);}

    template <class Data>
    constexpr auto success(P const &p, Data const &data) const {return success_of(p.implement(), data);}

    template <class ...Args>
    constexpr auto parse(P const &p, Args &&...args) const {return parse_of(p.implement(), std::forward<Args>(args)...);}
};

/******************************************************************************************/

}
