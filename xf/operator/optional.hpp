#pragma once
#include "../parse/parse.hpp"

namespace xf {

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

    template <class Tag, class Data, int_if<is_parse<Tag>> = 0>
    auto operator()(Tag tag, Data &&data) const {
        optional_type<decltype(parse(tag, subject, std::forward<Data>(data)))> ret;
        if (valid(subject, data)) ret = parse(tag, subject, std::forward<Data>(data));
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
