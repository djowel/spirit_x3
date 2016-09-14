#pragma once
#include "../parse/variant.hpp"
#include "../parse/parse.hpp"
#include <boost/hana/for_each.hpp>

namespace x4 {

/******************************************************************************************/

template <class V, class=void>
struct reduce_variant {
    template <class I, class T>
    constexpr auto operator()(I i, T &&t) const {return V(i, std::forward<T>(t));}
};

template <class T, class ...Ts>
struct reduce_variant<variant<T, Ts...>, std::enable_if_t<false && decltype(hana::tuple_t<T, Ts...> == hana::tuple_t<Ts..., T>)::value>> {
    template <class I, class T_>
    auto operator()(I, T_ &&t) const {return std::forward<T_>(t);}
};

/******************************************************************************************/

template <class P, class W>
struct alt_check {
    using value_type = decltype(std::declval<P const &>().check(std::declval<W &>()));
    value_type value;

    template <class ...Ts>
    alt_check(Ts &&...ts) : value(std::forward<Ts>(ts)...) {}
};

/******************************************************************************************/

template <class ...Parsers>
struct alternative : parser_base {
    hana::tuple<Parsers...> parsers;
    static constexpr auto types = hana::tuple_t<Parsers...>;

    template <class ...Ts>
    constexpr alternative(Ts &&...ts) : parsers(std::forward<Ts>(ts)...) {}


    template <class W>
    auto check_t(W &w) const {
        return hana::type_c<optional_variant<alt_check<Parsers, W>...>>;
    }

    template <class Window>
    auto check(Window &w) const -> decltype(*check_t(w)) {
        decltype(*check_t(w)) ret;
        hana::for_each(enumerate(parsers), [&](auto const &p) {
             if (ret) return;
             auto t = check_of(p[1_c], w);
             if (success_of(p[1_c], t)) ret.emplace(p[0_c], std::move(t));
        });
        return ret;
    }

    template <class Data, class ...Args>
    auto parse(Data &&data, Args &&...args) const {
        auto const f = [&](auto i) {return hana::decltype_(parse_of(parsers[i], std::move(data[i]), std::forward<Args>(args)...));};
        using R = decltype(*variant_c(indices_c<sizeof...(Parsers)> | f));

        return data.visit([&](auto i, auto &d) {
            return reduce_variant<R>()(i, parse_of(parsers[i], std::move(d.value), std::forward<Args>(args)...));
        });
    }
};

static constexpr auto alternative_c = hana::fuse(hana::template_<alternative>);

template <class T> struct is_alternative_t : std::false_type {};
template <class ...Parsers> struct is_alternative_t<alternative<Parsers...>> : std::true_type {};
template <class T> static constexpr auto is_alternative = hana::bool_c<is_alternative_t<T>::value>;

template <class T> constexpr auto make_alternative(T &&t) {
    return decltype(*alternative_c(types_of(t)))(std::forward<T>(t));
}

/******************************************************************************************/

template <class L, class R, int_if<(is_expression<L> || is_expression<R>) && !(is_alternative<L> || is_alternative<R>)> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::make_tuple(expr(l), expr(r)));}

template <class L, class R, int_if<is_alternative<L> && !is_alternative<R>> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::append(l.parsers, expr(r)));}

template <class L, class R, int_if<!is_alternative<L> && is_alternative<R>> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::prepend(r.parsers, expr(l)));}

template <class L, class R, int_if<is_alternative<L> && is_alternative<R>> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::concat(l.parsers, r.parsers));}

/******************************************************************************************/

template <class ...Ts>
constexpr auto any(Ts &&...ts) {return make_alternative(hana::make_tuple(expr(std::forward<Ts>(ts))...));}

/******************************************************************************************/

}
