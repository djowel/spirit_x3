#pragma once
#include "../parse/parse.hpp"

namespace x4 {

/******************************************************************************************/

template <class Subject>
class optional : parser_base {
    Subject subject;

public:

    constexpr optional(Subject s) : subject(std::move(s)) {}

    template <class Window>
    constexpr auto check(Window &w) const {return check_of(subject, w);}

    template <class Data>
    constexpr auto success(Data const &data) const {return hana::true_c;}

    template <class Data, class ...Args>
    auto parse(Data &&data, Args &&...args) const {
        optional_type<decltype(parse_of(subject, std::forward<Data>(data), std::forward<Args>(args)...))> ret;
        if (success_of(subject, data)) ret = parse_of(subject, std::forward<Data>(data), std::forward<Args>(args)...);
        return ret;
    }
};

/******************************************************************************************/

template <class T> struct is_optional : std::false_type {};
template <class T> struct is_optional<optional<T>> : std::true_type {};

template <class Subject, int_if<is_expression<Subject>> = 0>
auto operator~(Subject const &subject) {return optional<Subject>(subject);}

/******************************************************************************************/

}
