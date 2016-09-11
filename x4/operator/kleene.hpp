#pragma once

namespace x4 {

/******************************************************************************************/

template <class Subject>
struct kleene : expression_base {
    Subject subject;

    template <class Window>
    auto check(Window &w) const {
        decltype(*container(check_type(w)(subject))) ret;
        while (true) {
            ret.emplace_back(subject.check(w));
            if (!ret.back()) break;
        }
        ret.pop_back();
        return ret;
    }

    template <class Data>
    constexpr auto success(Data const &data) const {return hana::true_c;}

    template <class Data, class ...Args>
    auto parse(Data data, Args &&...args) const {
        decltype(*container(parse_type(data.front(), args...)(subject))) ret;
        ret.reserve(data.size());
        for (auto &&d : data) ret.emplace_back(parse_of(subject, std::move(d), args...));
        return ret;
    }
};

/******************************************************************************************/

static constexpr auto kleene_c = hana::template_<kleene>;

/******************************************************************************************/

template <class Subject, int_if<is_expression<Subject>> = 0>
constexpr auto operator*(Subject const &s) {return kleene<Subject>{s};}

}
