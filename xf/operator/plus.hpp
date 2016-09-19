#pragma once
#include "kleene.hpp"

namespace xf {

/******************************************************************************************/

template <class Subject>
class plus : kleene<Subject> {
    auto const & base() const {return static_cast<kleene<Subject> const &>(*this);}

public:

    template <class ...Ts>
    explicit constexpr plus(Ts &&...ts) : kleene<Subject>(std::forward<Ts>(ts)...) {}

    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    auto operator()(Tag tag, Window &w) const {return check(tag, base(), w);}

    template <class Data>
    auto operator()(valid_t, Data const &data) const {return !data.empty();}

    template <class Tag, class Data, int_if<is_parse<Tag>> = 0>
    auto operator()(Tag tag, Data &&data) const {return parse(tag, base(), std::forward<Data>(data));}

};

/******************************************************************************************/

static constexpr auto plus_c = hana::template_<plus>;

/******************************************************************************************/

template <class Subject, int_if<is_expression<Subject>> = 0>
constexpr auto operator+(Subject const &s) {return plus<Subject>(s);}

}
