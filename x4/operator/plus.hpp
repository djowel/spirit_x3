#pragma once
#include "kleene.hpp"

namespace x4 {

/******************************************************************************************/

template <class Subject>
struct plus : kleene<Subject> {

    template <class ...Ts> plus(Ts &&...ts) : kleene<Subject>(std::forward<Ts>(ts)...) {}

    template <class Data>
    auto success(Data const &data) const {return !data.empty();}
};

/******************************************************************************************/

static constexpr auto plus_c = hana::template_<plus>;

/******************************************************************************************/

template <class Subject, int_if<is_expression<Subject>> = 0>
constexpr auto operator+(Subject const &s) {return plus<Subject>(s);}

}
