#pragma once
#include "../operator/sequence.hpp"

namespace x4 {

/******************************************************************************************/

struct lexeme_t {
    template <class Subject>
    class expression : parser_base {
        Subject subject;

    public:

        constexpr expression(Subject s) : subject(std::move(s)) {}

        template <class Window>
        constexpr auto check(Window &w) const {return w.no_skip([&](auto &w) {return check_of(subject, w);});}

        template <class Data>
        constexpr auto success(Data const &data) const {return success_of(subject, data);}

        template <class Data, class ...Args>
        constexpr auto parse(Data data, Args &&...args) const {
            return parse_of(subject, std::move(data), std::forward<Args>(args)...);
        }
    };

    constexpr lexeme_t() {}

    template <class ...Ts, int_if<sizeof...(Ts) >= 2> = 0>
    constexpr auto operator()(Ts &&...ts) const {return expression<decltype(seq(std::forward<Ts>(ts)...))>(seq(std::forward<Ts>(ts)...));}

    template <class T>
    constexpr auto operator()(T &&t) const {return expression<std::decay_t<decltype(expr(std::forward<T>(t)))>>(expr(std::forward<T>(t)));}
};

static constexpr auto lexeme = lexeme_t();

/******************************************************************************************/

}
