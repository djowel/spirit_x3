#pragma once
#include "parse.hpp"

namespace xf {

/******************************************************************************************/

struct declaration : expression_base {constexpr declaration() {}};

template <class T>
struct declaration_t : expression_base {constexpr declaration_t() {}};

template <class T> static constexpr auto declaration_c = declaration_t<T>();

/******************************************************************************************/

void implement(...);

template <class P, class=void> struct has_adl_impl : std::false_type {};
template <class P> struct has_adl_impl<P, void_if<!(std::is_same<decltype(implement(std::declval<P const>())), void>::value)>> : std::true_type {};

template <class P, class=void> struct has_member_impl : std::false_type {};
template <class P> struct has_member_impl<P, void_if<!(std::is_same<decltype(std::declval<P const>().implement()), void>::value)>> : std::true_type {};

/******************************************************************************************/

template <class P>
struct implementation<P, void_if<has_adl_impl<P>::value && !has_member_impl<P>::value>> {
    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    constexpr auto operator()(Tag tag, P const &p, Window &w) const {return check(tag, implement(p), w);}

    template <class Data>
    constexpr auto operator()(valid_t, P const &p, Data const &data) const {return valid(implement(p), data);}

    template <class Tag, class Data, int_if<is_parse<Tag>> = 0>
    constexpr auto operator()(Tag tag, P const &p, Data &&data) const {return parse(tag, implement(p), std::forward<Data>(data));}
};

/******************************************************************************************/

template <class P>
struct implementation<P, void_if<has_member_impl<P>::value>> {
    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    constexpr auto operator()(Tag tag, P const &p, Window &w) const {return check(tag, p.implement(), w);}

    template <class Data>
    constexpr auto operator()(valid_t, P const &p, Data const &data) const {return valid(p.implement(), data);}

    template <class Tag, class Data, int_if<is_parse<Tag>> = 0>
    constexpr auto operator()(Tag tag, P const &p, Data &&data) const {return parse(tag, p.implement(), std::forward<Data>(data));}
};

/******************************************************************************************/

#define XF_DECLARE(NAME) static auto constexpr NAME = ::xf::declaration_c<class NAME>;

#define XF_DEFINE(NAME) \
template <class T> struct implementation; \
template <> struct implementation<std::decay_t<decltype(NAME)>> {template <bool=true> static constexpr bool value = false;}; \
template <bool B=true> constexpr auto implement(std::decay_t<decltype(NAME)>) {return implementation<std::decay_t<decltype(NAME)>>::value<B>;} \
template <> auto const implementation<std::decay_t<decltype(NAME)>>::value<>

}
