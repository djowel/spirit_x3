#pragma once
#include "../parse/parse.hpp"
#include "../parse/common.hpp"
#include <boost/hana/back.hpp>

namespace x4 {

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

    template <class Tag, class Data, class ...Args, int_if<is_parse<Tag>> = 0>
    auto operator()(Tag tag, Data &&data, Args &&...args) const {
        return no_void[visit_expression<Subject>()(tag, subject, operation, data, std::forward<Args>(args)...), no_void];
    }
};

static constexpr auto transform_c = hana::template_<transform>;

/******************************************************************************************/

template <class L, class R, int_if<is_expression<L> || is_expression<R>> = 0>
constexpr auto operator%(L const &l, R const &r) {return transform<L, R>(l, r);}

/******************************************************************************************/

}
