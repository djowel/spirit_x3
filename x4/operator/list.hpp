#pragma once
#include "plus.hpp"

namespace x4 {

/******************************************************************************************/

// replace vector<void> with size_t probably

template <class Subject, class Separator>
class list : public plus<Subject> {
    Subject subject;
    Separator separator;

public:

    constexpr list(Subject sub, Separator sep) : subject(std::move(sub)), separator(std::move(sep)) {}

    template <class Window>
    auto check(Window &w) const {
        container_type<decltype(*check_type(w)(subject))> ret;
        append(ret, check_of(subject, w));
        while (success_of(separator, check_of(separator, w)) && success_of(subject, ret.back()))
            append(ret, check_of(subject, w));
        ret.pop_back();
        return ret;
    }
};

static constexpr auto list_c = hana::template_<list>;

template <class L, class R>
auto make_list(L &&l, R &&r) {return list<std::decay_t<L>, std::decay_t<R>>(std::forward<L>(l), std::forward<R>(r));}

/******************************************************************************************/

template <class L, class R, int_if<is_expression<L> || is_expression<R>> = 0>
constexpr auto operator/(L const &l, R const &r) {return make_list(expression(l), expression(r));}

/******************************************************************************************/

}
