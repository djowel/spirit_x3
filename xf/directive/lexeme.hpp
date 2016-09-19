#pragma once
#include "../operator/sequence.hpp"

namespace xf {

/******************************************************************************************/

struct lexeme_t {
    template <class Subject>
    class expression : parser_base {
        Subject subject;

    public:

        explicit constexpr expression(Subject s) : subject(std::move(s)) {}

        template <class T, class Window, int_if<is_check<T>> = 0>
        constexpr auto operator()(T tag, Window &w) const {return w.no_skip([&](auto &w) {return check(tag, subject, w);});}

        template <class Data>
        constexpr auto operator()(valid_t, Data const &data) const {return valid(subject, data);}

        template <class T, class Data, int_if<is_parse<T>> = 0>
        constexpr auto operator()(T tag, Data data) const {
            return parse(tag, subject, std::move(data));
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
