#pragma once
#include "../parse/parse.hpp"

namespace x4 {

/******************************************************************************************/

template <class Subject>
class optional : parser_base {
    Subject subject;

public:

    constexpr optional(Subject s) : subject(std::move(s)) {}

    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    auto operator()(Tag tag, Window &w) const {return check(tag, subject, w);}

    template <class Data>
    auto operator()(valid_t, Data const &data) const {return hana::true_c;}

    template <class Tag, class Data, class ...Args, int_if<is_check<Tag>> = 0>
    auto operator()(Tag tag, Data &&data, Args &&...args) const {
        optional_type<decltype(parse(subject, std::forward<Data>(data), std::forward<Args>(args)...))> ret;
        if (valid(subject, data)) ret = parse(tag, subject, std::forward<Data>(data), std::forward<Args>(args)...);
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
