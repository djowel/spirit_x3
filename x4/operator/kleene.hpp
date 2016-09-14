#pragma once
#include "../parse/parse.hpp"
#include "../parse/common.hpp"

namespace x4 {

/******************************************************************************************/

// replace vector<void> with size_t probably

template <class Subject>
class kleene : parser_base {
    Subject m_subject;

public:

    decltype(auto) subject() const {return m_subject;}

    constexpr kleene(Subject s) : m_subject(std::move(s)) {}

    template <class Window>
    auto check(Window &w) const {
        container_type<decltype(*check_type(w)(subject()))> ret;
        append(ret, check_of(subject(), w));
        while (success_of(subject(), ret.back())) append(ret, check_of(subject(), w));
        ret.pop_back();
        return ret;
    }

    template <class Data>
    constexpr auto success(Data const &data) const {return hana::true_c;}

    template <class Data, class ...Args>
    auto parse(Data data, Args &&...args) const {
        container_type<decltype(parse_of(subject(), std::move(data.front()), args...))> ret;
        ret.reserve(data.size());
        for (auto &&d : data) append(ret, parse_of(subject(), std::move(d), args...));
        return ret;
    }
};

/******************************************************************************************/

static constexpr auto kleene_c = hana::template_<kleene>;

/******************************************************************************************/

template <class Subject, int_if<is_expression<Subject>> = 0>
constexpr auto operator*(Subject const &s) {return kleene<Subject>(s);}

}
