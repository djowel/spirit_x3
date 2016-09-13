#pragma once
#include "../parse/common.hpp"

namespace x4 {

/******************************************************************************************/

struct lexeme_t {
    template <class Subject>
    class expression : expression_base {
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

    template <class Subject>
    constexpr auto operator[](Subject &&s) {return expression<std::decay_t<Subject>>(std::forward<Subject>(s));}
};

static constexpr auto lexeme = lexeme_t();

/******************************************************************************************/

}
