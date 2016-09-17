#pragma once
#include "window.hpp"
#include "../support/counter.hpp"
#include <boost/hana/type.hpp>
#include <boost/hana/functional/overload_linearly.hpp>

#include <stdexcept>

namespace x4 {
namespace hana = boost::hana;

/******************************************************************************************/

struct valid_t {};

static constexpr auto valid_c = valid_t();

/******************************************************************************************/

template <class P, class=void>
struct implementation {static_assert(P::has_an_implementation, "No implementation found");};

template <class Tag, class P, class Window>
auto check(Tag tag, P const &p, Window &w) {
    static_assert(is_check<Tag>, "Should be parsing tag");
    return no_void[implementation<P>()(tag, p, w), no_void];
}

template <class P, class Data>
auto valid(P const &p, Data const &data) {return implementation<P>()(valid_c, p, data);}

template <class Tag, class P, class ...Ts>
auto parse(Tag tag, P const &p, Ts &&...ts) {
    static_assert(is_parse<Tag>, "Should be parsing tag");
    return no_void[implementation<P>()(tag, p, std::forward<Ts>(ts)...), no_void];
}

/******************************************************************************************/

template <class T>
struct implementation<T, void_if<is_parser<T>>> {
    template <class Tag, class Parser, class Window, int_if<is_check<Tag>> = 0>
    auto operator()(Tag tag, Parser const &parser, Window &w) const {return parser(tag, w);}

    template <class Parser, class Data>
    auto operator()(valid_t, Parser const &parser, Data const &data) const {
        static auto const f1 = [](auto const &p, auto const &d) -> decltype(p(valid_c, d)) {return p(valid_c, d);};
        static auto const f2 = [](auto const &, auto const &d) -> decltype(std::enable_if_t<std::is_constructible<bool, decltype(d)>::value, bool>()) {return bool(d);};
        return hana::overload_linearly(f1, f2)(parser, data);
    }

    template <class Tag, class Parser, class Data, class ...Args, int_if<is_parse<Tag>> = 0>
    auto operator()(Tag tag, Parser const &p, Data &&d, Args &&...args) const {return p(tag, std::forward<Data>(d), std::forward<Args>(args)...);}
};

/******************************************************************************************/

template <class Subject, class ...Masks>
class parser_t {
    Subject subject;
    hana::tuple<Masks...> masks;
public:

    constexpr explicit parser_t(Subject s, Masks ...ms) : subject(std::move(s)), masks(std::move(ms)...) {}

    template <class V>
    auto match(V const &v) const {
        auto window = with_masks(make_window(v), masks);
        auto data = check(check_c, subject, window);
        return valid(subject, data);
    }

    template <class V, class ...Ts>
    decltype(auto) operator()(V const &v, Ts &&...ts) const {
        auto window = with_masks(make_window(v), masks);
        auto data = check(check_c, subject, window);
        if (!valid(subject, data)) throw std::runtime_error("parsing failed");
        return parse(parse_c, subject, std::move(data));
    }
};

template <class Subject, class ...Masks>
constexpr auto parser(Subject subject, Masks ...masks) {
    return parser_t<Subject, Masks...>(std::move(subject), std::move(masks)...);
}

/******************************************************************************************/

template <class Subject, class=void>
struct visit_expression {
    template <class Tag, class Data, class Operation, class ...Args>
    auto operator()(Tag tag, Subject const &s, Operation const &op, Data &&data, Args &&...args) const {
        auto f = [&](auto &&...ts) {return parse(tag, s, data, std::forward<decltype(ts)>(ts)...);};
        return op(f, std::forward<Args>(args)...);
    }
};

/******************************************************************************************/

}
