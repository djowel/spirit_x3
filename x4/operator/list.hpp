#pragma once
#include "plus.hpp"

namespace x4 {

/******************************************************************************************/

// replace vector<void> with size_t probably

template <class Subject, class Separator>
class list : public plus<Subject> {
    Separator separator;

public:

    using plus<Subject>::subject;

    constexpr list(Subject sub, Separator sep) : plus<Subject>(std::move(sub)), separator(std::move(sep)) {}

    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    auto check(Tag tag, Window &w) const {
        container_type<decltype(check(tag, subject(), w))> ret;
        append(ret, check(tag, subject(), w));
        while (valid(separator, check(tag, separator, w)) && valid(subject(), ret.back()))
            append(ret, check(tag, subject(), w));
        ret.pop_back();
        return ret;
    }
};

static constexpr auto list_c = hana::template_<list>;

template <class L, class R>
auto make_list(L &&l, R &&r) {return list<std::decay_t<L>, std::decay_t<R>>(std::forward<L>(l), std::forward<R>(r));}

/******************************************************************************************/

template <class L, class R, int_if<is_expression<L> || is_expression<R>> = 0>
constexpr auto operator/(L const &l, R const &r) {return make_list(expr(l), expr(r));}

/******************************************************************************************/

}
