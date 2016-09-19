#pragma once
#include "../parse/parse.hpp"
#include "../support/common.hpp"
#include <typeinfo>

#include <boost/hana/back.hpp>
#include <boost/hana/append.hpp>

namespace xf {

/******************************************************************************************/

template <class ...Parsers>
class sequence : public parser_base {
    hana::tuple<Parsers...> m_parsers;

    template <class T, class W>
    auto check_type(T tag, W &w) const {
        auto f = [&](auto const &p) {return hana::decltype_(check(tag, p, w));};
        return optional_c(tuple_c(hana::transform(m_parsers, f)));
    }

public:

    auto const & parsers() const {return m_parsers;}

    constexpr sequence(hana::tuple<Parsers...> tuple) : m_parsers(std::move(tuple)) {}

    template <class T, class Window, int_if<is_check<T>> = 0>
    auto operator()(T tag, Window &w) const {
        decltype(*check_type(tag, w)) ret;
        bool good = true;
        auto save = w;
        ret = hana::transform(m_parsers, [&](auto const &p) {
            decltype(check(tag, p, w)) t;
            if (good) good = valid(p, t = check(tag, p, w));
            return t;
        });
        if (!good) {ret.reset(); w = save;}
        return ret;
    }

    template <class T, class Data, int_if<is_parse<T>> = 0>
    auto operator()(T tag, Data &&data) const {
        return hana::transform(indices_c<sizeof...(Parsers)>, [&] (auto i) {
            return parse(tag, m_parsers[i], std::move((*data)[i]));
        });
    }
};

static constexpr auto sequence_c = hana::fuse(hana::template_<sequence>);

template <class T> constexpr auto make_sequence(T tuple) {
    return decltype(*sequence_c(types_of(tuple)))(std::move(tuple));
}

template <class T, class=void> struct is_sequence_t : std::false_type {};
template <class ...Parsers> struct is_sequence_t<sequence<Parsers...>, void> : std::true_type {};

template <class T> static constexpr auto is_sequence = hana::bool_c<is_sequence_t<T>::value>;

/******************************************************************************************/

template <class L, class R, int_if<(is_expression<L> || is_expression<R>) && !(is_sequence<L> || is_sequence<R>)> = 0>
constexpr auto operator>>(L const &l, R const &r) {return make_sequence(hana::make_tuple(expr(l), expr(r)));}

template <class L, class R, int_if<is_sequence<L> && !is_sequence<R>> = 0>
constexpr auto operator>>(L const &l, R const &r) {return make_sequence(hana::append(l.parsers(), expr(r)));}

template <class L, class R, int_if<!is_sequence<L> && is_sequence<R>> = 0>
constexpr auto operator>>(L const &l, R const &r) {return make_sequence(hana::prepend(r.parsers(), expr(l)));}

template <class L, class R, int_if<is_sequence<L> && is_sequence<R>> = 0>
constexpr auto operator>>(L const &l, R const &r) {return make_sequence(hana::concat(l.parsers(), r.parsers()));}

template <class ...Ts>
constexpr auto seq(Ts &&...ts) {return make_sequence(hana::make_tuple(expr(std::forward<Ts>(ts))...));}

/******************************************************************************************/

template <class ...Parsers>
struct apply_t<sequence<Parsers...>, void> {
    template <class Tag, class Data, class Operation>
    auto operator()(Tag tag, sequence<Parsers...> const &s, Operation const &op, Data &&data) {
        return hana::unpack(hana::transform(indices_c<sizeof...(Parsers)>, [&](auto i) {
            // need to apply here ? : apply<>(tag, s.parsers()[i], op, std::move((*data)[i]));
            return parse(tag, s.parsers()[i], std::move((*data)[i]));
        }), op);
    }
};

/******************************************************************************************/

}
