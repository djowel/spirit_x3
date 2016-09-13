#pragma once
#include "../window/window.hpp"

#include <boost/hana/type.hpp>
#include <boost/hana/functional/overload_linearly.hpp>

#include <stdexcept>

namespace x4 {
namespace hana = boost::hana;

/******************************************************************************************/

template <class P, class=void>
struct implementation {static_assert(P::has_an_implementation, "No implementation found");};

template <class P, class Window>
constexpr auto check_of(P const &p, Window &w) {
    return no_void*(implementation<P>().check(p, w), no_void);
}

template <class P, class Data>
constexpr auto success_of(P const &p, Data const &data) {return implementation<P>().success(p, data);}

template <class P, class ...Ts>
constexpr auto parse_of(P const &p, Ts &&...ts) {
    return no_void*(implementation<P>().parse(p, std::forward<Ts>(ts)...), no_void);
}

/******************************************************************************************/

template <class T>
struct implementation<T, void_if<is_expression<T>>> {
    template <class Parser, class Window>
    constexpr auto check(Parser const &parser, Window &w) const {return parser.check(w); }

    template <class Parser, class Data>
    auto success(Parser const &parser, Data const &data) const {
        static auto const f1 = [](auto const &p, auto const &d) -> decltype(p.success(d)) {return p.success(d);};
        static auto const f2 = [](auto const &, auto const &d) -> decltype(std::enable_if_t<std::is_constructible<bool, decltype(d)>::value, bool>()) {return bool(d);};
        return hana::overload_linearly(f1, f2)(parser, data);
    }

    template <class Parser, class Data, class ...Args>
    auto parse(Parser const &p, Data &&d, Args &&...args) const {return p.parse(std::forward<Data>(d));}
};

/******************************************************************************************/

template <class Window>
auto check_type(Window &) {
    return [](auto const &parser) {return hana::decltype_(check_of(parser, declref<Window>()));};
}

/******************************************************************************************/

template <class Subject, class ...Masks>
class parser_t {
    Subject subject;
    hana::tuple<Masks...> masks;
public:

    constexpr explicit parser_t(Subject s, Masks ...ms) : subject(std::move(s)), masks(std::move(ms)...) {}

    template <class V, class ...Ts>
    constexpr decltype(auto) operator()(V const &v, Ts &&...ts) {
        auto window = with_masks(make_window(v), masks);
        auto data = check_of(subject, window);
        if (!success_of(subject, data)) throw std::runtime_error("parsing failed");
        return parse_of(subject, std::move(data));
    }
};

template <class Subject, class ...Masks>
constexpr auto parser(Subject subject, Masks ...masks) {
    return parser_t<Subject, Masks...>(std::move(subject), std::move(masks)...);
}

/******************************************************************************************/

template <class Subject, class=void>
struct visit_expression {
    template <class Data, class Operation, class ...Args>
    auto operator()(Subject const &s, Operation const &op, Data &&data, Args &&...args) {
        auto f = [&](auto &&...ts) {return parse_of(s, data, std::forward<decltype(ts)>(ts)...);};
        return op(f, std::forward<Args>(args)...);
    }
};

/******************************************************************************************/

}
