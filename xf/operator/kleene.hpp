#pragma once
#include "../parse/parse.hpp"
#include "../support/common.hpp"

namespace xf {

/******************************************************************************************/

// replace vector<void> with size_t probably

template <class Subject>
class kleene : parser_base {
    Subject m_subject;

public:

    decltype(auto) subject() const {return m_subject;}

    constexpr kleene(Subject s) : m_subject(std::move(s)) {}

    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    auto operator()(Tag tag, Window &w) const {
        container_type<decltype(check(tag, subject(), w))> ret;
        append(ret, check(tag, subject(), w));
        while (valid(subject(), ret.back())) append(ret, check(tag, subject(), w));
        ret.pop_back();
        return ret;
    }

    template <class Data>
    constexpr auto operator()(valid_t, Data const &data) const {return hana::true_c;}

    template <class Tag, class Data, int_if<is_parse<Tag>> = 0>
    auto operator()(Tag tag, Data data) const {
        container_type<decltype(parse(tag, subject(), std::move(data.front())))> ret;
        ret.reserve(data.size());
        for (auto &&d : data) append(ret, parse(tag, subject(), std::move(d)));
        return ret;
    }
};

/******************************************************************************************/

static constexpr auto kleene_c = hana::template_<kleene>;

/******************************************************************************************/

template <class Subject, int_if<is_expression<Subject>> = 0>
constexpr auto operator*(Subject const &s) {return kleene<Subject>(s);}

}
