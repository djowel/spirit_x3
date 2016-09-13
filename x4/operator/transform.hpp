#pragma once
#include "../parse/parse.hpp"
#include "../parse/common.hpp"
#include <boost/hana/back.hpp>

namespace x4 {

/******************************************************************************************/

template <class Subject, class Operation>
struct transform : expression_base {
    Subject subject;
    Operation operation;

    transform(Subject s, Operation op) : subject(std::move(s)), operation(std::move(op)) {}

    template <class Window>
    auto check(Window &w) const {return check_of(subject, w);}

    template <class Data>
    auto success(Data const &d) const {return success_of(subject, d);}

    template <class Data, class ...Args>
    auto parse(Data &&data, Args &&...args) const {
        return no_void[visit_expression<Subject>()(subject, operation, data, std::forward<Args>(args)...), no_void];
    }
};

static constexpr auto transform_c = hana::template_<transform>;

/******************************************************************************************/

template <class L, class R, int_if<is_expression<L> || is_expression<R>> = 0>
constexpr auto operator%(L const &l, R const &r) {return transform<L, R>(l, r);}

/******************************************************************************************/

}
