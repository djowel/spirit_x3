#pragma once
#include "../parse/parse.hpp"
#include "../parse/common.hpp"
#include <boost/hana/back.hpp>

namespace x4 {

/******************************************************************************************/

template <class ...Parsers>
struct sequence : expression_base {
    hana::tuple<Parsers...> parsers;

    template <class ...Ts>
    constexpr sequence(Ts &&...ts) : parsers(std::forward<Ts>(ts)...) {}

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

static constexpr auto sequence_c = hana::template_<sequence>;

/******************************************************************************************/

template <class L, class R, int_if<is_expression<L> || is_expression<R>> = 0>
constexpr auto operator>>(L const &l, R const &r) {return sequence<L, R>(l, r);}

/******************************************************************************************/

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
                return [&, i](auto &&...ts) {return parse_of(s.parsers[i], (*data)[i], std::forward<decltype(ts)>(ts)...);};
        }), op, std::forward<Args>(args)...);
    }
};

/******************************************************************************************/

}
