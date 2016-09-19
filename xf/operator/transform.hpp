#pragma once
#include "../parse/parse.hpp"
#include "../support/common.hpp"

#include <boost/hana/back.hpp>

namespace xf {

/******************************************************************************************/

template <class Subject, class Operation>
struct transform : parser_base {
    Subject subject;
    Operation operation;

    transform(Subject s, Operation op) : subject(std::move(s)), operation(std::move(op)) {}

    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    auto operator()(Tag tag, Window &w) const {return check(tag, subject, w);}

    template <class Data>
    auto operator()(valid_t, Data const &d) const {return valid(subject, d);}

    template <class Tag, class Data, int_if<is_parse<Tag>> = 0>
    auto operator()(Tag tag, Data &&data) const {
        return no_void[apply(tag, subject, operation, data), no_void];
    }
};

static constexpr auto transform_c = hana::template_<transform>;

/******************************************************************************************/

template <class T, class=void>
struct prevent_transform : std::false_type {};

template <class L, class R, int_if<(is_expression<L> || is_expression<R>) && !prevent_transform<L>::value && !prevent_transform<R>::value> = 0>
constexpr auto operator%(L const &l, R const &r) {return transform<L, R>(l, r);}

/******************************************************************************************/

}
