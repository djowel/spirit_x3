#pragma once
#include "../operator/sequence.hpp"
#include "infix.hpp"
#include <boost/hana/functional/capture.hpp>

namespace xf {

/******************************************************************************************/

template <class Subject, class Operation>
class partial_t : parser_base {
    Subject subject;
    Operation operation;

public:

    constexpr partial_t(Subject s, Operation o) : subject(std::move(s)), operation(std::move(o)) {}

    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    auto operator()(Tag tag, Window &w) const {return check(tag, subject, w);}

    template <class Data>
    auto operator()(valid_t, Data const &d) const {return valid(subject, d);}

    template <class Tag, class Data, int_if<is_parse<Tag>> = 0>
    auto operator()(Tag tag, Data &&data) const {
        return no_void[apply(tag, subject, hana::capture, std::forward<Data>(data))(operation), no_void];
    }
};

/******************************************************************************************/

struct partial_gen : infix<partial_gen> {
    template <class Subject, class Operation>
    constexpr auto operator()(Subject &&sub, Operation &&op) const {
        return partial_t<std::decay_t<Subject>, std::decay_t<Operation>>(std::forward<Subject>(sub), std::forward<Operation>(op));
    }
};

static constexpr auto partial = partial_gen{};

/******************************************************************************************/

}
