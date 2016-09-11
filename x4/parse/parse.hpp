#pragma once
#include "../window/window.hpp"

#include <boost/hana/type.hpp>
#include <boost/hana/functional/overload_linearly.hpp>

#include <stdexcept>

namespace x4 {
namespace hana = boost::hana;

template <class Parser, class Window>
constexpr auto check_of(Parser const &parser, Window &w) {return parser.check(w); }

template <class Window>
auto check_type(Window &) {
    return [](auto const &parser) {return hana::decltype_(check_of(parser, std::declval<Window &>()));};
}

/******************************************************************************************/

template <class Parser, class Data>
auto success_of(Parser const &parser, Data const &data) {
    static auto const f1 = [](auto const &p, auto const &d) -> decltype(p.success(d)) {return p.success(d);};
    static auto const f2 = [](auto const &, auto const &d) -> decltype(std::enable_if_t<std::is_constructible<bool, decltype(d)>::value, bool>()) {return bool(d);};
    return hana::overload_linearly(f1, f2)(parser, data);
}

/******************************************************************************************/

template <class V, class Parser, class ...Args>
auto parse(V const &v, Parser const &p, Args &&...args) {
    auto w = make_window(v);
    auto data = p.check(w);
    if (!success_of(p, data)) throw std::runtime_error("parsing failed");
    return p.parse(std::move(data));
}

template <class Parser, class Data, class ...Args>
auto parse_of(Parser const &p, Data d, Args &&...args) {return p.parse(d);}

template <class ...Args>
auto parse_type(Args &&...args) {
    return [](auto const &parser) {return hana::decltype_(parse_of(parser, std::declval<Args &&>()...));};
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
