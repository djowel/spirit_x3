#pragma once
#include "../parse/parse.hpp"
#include "../parse/variant.hpp"
#include "../parse/common.hpp"

namespace x4 {

/******************************************************************************************/

template <class Subject, class ...Fails>
struct restriction : expression_base {
    Subject subject;
    hana::tuple<Fails...> fails;

    template <class ...Ts>
    constexpr restriction(Subject s, Ts &&...ts) : subject(std::move(s)), fails(std::forward<Ts>(ts)...) {}

    template <class Window>
    auto check(Window &w) const {
        auto w_mod = w;
        constexpr auto ok = hana::none_of(fails, [&](auto const &f) {return success_of(check_of(f, w_mod));});
        optional_type<decltype(*check_type(w)(subject))> ret;
        if (!ok) return ret;
        ret.emplace(check_of(subject, w));
        if (!success_of(subject, *ret)) ret.reset();
        return ret;
    }

    template <class Data, class ...Args>
    auto parse(Data data, Args &&...args) const {
        return parse_of(subject, std::move(*data), std::forward<Args>(args)...);
    }
};

/******************************************************************************************/

static constexpr auto restriction_c = hana::fuse(hana::template_<restriction>);

template <class T> struct is_restriction : std::false_type {};
template <class Subject, class ...Fails>
struct is_restriction<restriction<Subject, Fails...>> : std::true_type {};

template <class S, class T>
auto make_restriction(S &&s, T &&t) {
    return decltype(*restriction_c(types_in(hana::prepend(t, s))))(std::forward<S>(s), std::forward<T>(t));
}

/******************************************************************************************/

template <class Subject, class Fail, int_if<(is_expression<Subject> || is_expression<Fail>) && !is_restriction<Subject>::value> = 0>
constexpr auto operator-(Subject const &s, Fail const &f) {return restriction<Subject, Fail>(expression(s), hana::make_tuple(expression(f)));}

template <class Subject, class Fail, class ...Fails>
constexpr auto operator-(restriction<Subject, Fails...> const &r, Fail const &f) {
    return restriction<Subject, Fails..., Fail>(r.subject, hana::append(r.fails, expression(f)));
}

/******************************************************************************************/

}

