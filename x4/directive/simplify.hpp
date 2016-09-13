#pragma once
#include "../operator/sequence.hpp"

namespace x4 {

/******************************************************************************************/

template <class Subject>
struct simplified_t : expression_base {
    Subject subject;

    constexpr simplified_t(Subject s) : subject(std::move(s)) {}

    template <class Window>
    auto check(Window &w) const {return check_of(subject, w);}

    template <class Data>
    auto success(Data const &d) const {return success_of(subject, d);}

    template <class Data, class ...Args>
    auto parse(Data &&data, Args &&...args) const {
        return parse_of(subject, std::forward<Data>(data), std::forward<Args>(args)...);
    }
};

/******************************************************************************************/

struct simplify_t {};

static constexpr auto simplify = simplify_t{};

template <class Subject>
constexpr auto operator%(Subject &&subject, simplify_t) {
    return simplified_t<std::decay_t<Subject>>{std::forward<Subject>(subject)};
}

/******************************************************************************************/

template <class Subject>
struct visit_expression<simplified_t<Subject>, void_if<!is_sequence<Subject>>> {
    template <class Data, class Operation, class ...Args>
    auto operator()(simplified_t<Subject> const &s, Operation const &op, Data &&data, Args &&...args) const {
        return op(parse_of(s, std::forward<Data>(data)), std::forward<Args>(args)...);
    }
};

template <class Subject>
struct visit_expression<simplified_t<Subject>, void_if<is_sequence<Subject>>> {
    template <class ...Args>
    struct helper {
        template <std::size_t ...Is, class T, class Op>
        constexpr auto operator()(std::index_sequence<Is...>, T t, Op const &op, Args &&...args) const {
            return op(std::move(hana::at_c<Is>(t))..., std::forward<Args>(args)...);
        }
    };

    template <class Data, class Operation, class ...Args>
    auto operator()(simplified_t<Subject> const &s, Operation const &op, Data &&data, Args &&...args) const {
        constexpr auto is = std::make_index_sequence<decltype(hana::length(parse_of(s, std::forward<Data>(data))))::value>();
        return helper<Args...>()(is, parse_of(s, std::forward<Data>(data)), op, std::forward<Args>(args)...);
    }
};

/******************************************************************************************/

}
