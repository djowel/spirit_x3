#pragma once
#include "../parse/parse.hpp"
#include "../parse/common.hpp"

#include <boost/hana/back.hpp>
#include <boost/hana/append.hpp>

namespace x4 {

/******************************************************************************************/

template <class ...Parsers>
struct sequence : expression_base {
    hana::tuple<Parsers...> parsers;

    constexpr sequence(hana::tuple<Parsers...> tuple) : parsers(std::move(tuple)) {}

    template <class Window>
    auto check(Window &w) const {
        decltype(*optional_c(tuple_c(hana::transform(parsers, check_type(w))))) ret;
        bool good = true;
        auto save = w;
        ret = hana::transform(parsers, [&](auto const &p) {
            decltype(*check_type(w)(p)) t;
            if (good) good = success_of(p, t = check_of(p, w));
            return t;
        });
        if (!good) {ret.reset(); w = save;}
        return ret;
    }

    template <class Data, class ...Args>
    auto parse(Data &&data, Args &&...args) const {
        return hana::transform(indices_c<sizeof...(Parsers)>, [&] (auto i) {
            return parse_of(parsers[i], std::move((*data)[i]), args...);
        });
    }
};

static constexpr auto sequence_c = hana::fuse(hana::template_<sequence>);

template <class T> constexpr auto make_sequence(T tuple) {
    return decltype(*sequence_c(types_in(tuple)))(std::move(tuple));
}

template <class T, class=void> struct is_sequence_t : std::false_type {};
template <class ...Parsers> struct is_sequence_t<sequence<Parsers...>, void> : std::true_type {};

template <class T> static constexpr auto is_sequence = hana::bool_c<is_sequence_t<T>::value>;

/******************************************************************************************/

template <class L, class R, int_if<(is_expression<L> || is_expression<R>) && !(is_sequence<L> || is_sequence<R>)> = 0>
constexpr auto operator>>(L const &l, R const &r) {return make_sequence(hana::make_tuple(expr(l), expr(r)));}

template <class L, class R, int_if<is_sequence<L> && !is_sequence<R>> = 0>
constexpr auto operator>>(L const &l, R const &r) {return make_sequence(hana::append(l.parsers, expr(r)));}

template <class L, class R, int_if<!is_sequence<L> && is_sequence<R>> = 0>
constexpr auto operator>>(L const &l, R const &r) {return make_sequence(hana::prepend(expr(l), r.parsers));}

template <class L, class R, int_if<is_sequence<L> && is_sequence<R>> = 0>
constexpr auto operator>>(L const &l, R const &r) {return make_sequence(hana::concat(l.parsers, r.parsers));}

/******************************************************************************************/

template <class Index, class Parser, class Data>
struct sequence_caller {
    Index index;
    Parser parser;
    Data data;

    template <class ...Ts>
    auto operator()(Ts &&...ts) const & {return parse_of(parser, data, std::forward<Ts>(ts)...);}

    template <class ...Ts>
    auto operator()(Ts &&...ts) && {return parse_of(parser, std::move(data), std::forward<Ts>(ts)...);}

    template <class T, int_if<(std::is_convertible<decltype(parse_of(parser, data)), T>::value)> = 0>
    operator T() const & {return parse_of(parser, data);}
    template <class T, int_if<(std::is_convertible<decltype(parse_of(parser, std::move(data))), T>::value)> = 0>
    operator T() && {return parse_of(parser, std::move(data));}
};

template <class ...Parsers>
struct visit_expression<sequence<Parsers...>, void> {
    template <class ...Args>
    struct helper {
        template <class Fs, class Operation, std::size_t ...Is>
        auto operator()(std::index_sequence<Is...>, Fs fs, Operation const &op, Args &&...args) const {
            return op(std::move(hana::at_c<Is>(fs))..., std::forward<Args>(args)...);
        }
    };

    template <class Data, class Operation, class ...Args>
    auto operator()(sequence<Parsers...> const &s, Operation const &op, Data &&data, Args &&...args) {
        return helper<Args...>()(std::make_index_sequence<sizeof...(Parsers)>(),
            hana::transform(indices_c<sizeof...(Parsers)>, [&](auto i) {
                return sequence_caller<decltype(i), decltype(s.parsers[i]), std::decay_t<decltype((*data)[i])>>{i, s.parsers[i], std::move((*data)[i])};
            }), op, std::forward<Args>(args)...);
    }
};

/******************************************************************************************/

}
