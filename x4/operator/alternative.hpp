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

// need to replace voids in variant with nothing, then it should be OK

template <class ...Parsers>
struct alternative : expression_base {
    hana::tuple<Parsers...> parsers;

    template <class ...Ts>
    constexpr alternative(Ts &&...ts) : parsers(std::forward<Ts>(ts)...) {}

    template <class Window>
    auto check(Window &w) const {
        decltype(*optional_variant_c(hana::transform(parsers, check_type(w)))) ret;
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
        using R = decltype(*variant_c(hana::transform(indices_c<sizeof...(Parsers)>, f)));

        return data.visit([&](auto i, auto &d) {
            return reduce_variant<R>()(i, parse_of(parsers[i], std::move(d), std::forward<Args>(args)...));
        });
    }
};

static constexpr auto alternative_c = hana::fuse(hana::template_<alternative>);

template <class T> struct is_alternative_t : std::false_type {};
template <class ...Parsers> struct is_alternative_t<alternative<Parsers...>> : std::true_type {};
template <class T> static constexpr auto is_alternative = hana::bool_c<is_alternative_t<T>::value>;

template <class T> constexpr auto make_alternative(T &&t) {
    return decltype(*alternative_c(types_in(t)))(std::forward<T>(t));
}

/******************************************************************************************/

template <class L, class R, int_if<(is_expression<L> || is_expression<R>) && !(is_alternative<L> || is_alternative<R>)> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::make_tuple(expression(l), expression(r)));}

template <class L, class R, int_if<is_alternative<L> && !is_alternative<R>> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::append(l.parsers, expression(r)));}

template <class L, class R, int_if<!is_alternative<L> && is_alternative<R>> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::prepend(r.parsers, expression(l)));}

template <class L, class R, int_if<is_alternative<L> && is_alternative<R>> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::concat(l.parsers, r.parsers));}

/******************************************************************************************/

}
