#pragma once
#include "../operator/sequence.hpp"

namespace x4 {

/******************************************************************************************/

template <class Subject>
struct simplified_t : parser_base {
    Subject subject;

    explicit constexpr simplified_t(Subject s) : subject(std::move(s)) {}

    template <class T, class Window, int_if<is_check<T>> = 0>
    auto operator()(T t, Window &w) const {return check(t, subject, w);}

    template <class Data>
    auto operator()(valid_t, Data const &d) const {return valid(subject, d);}

    template <class T, class Data, class ...Args, int_if<is_parse<T>> = 0>
    auto operator()(T t, Data &&data, Args &&...args) const {
        return parse(subject, std::forward<Data>(data), std::forward<Args>(args)...);
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
        return op(parse(s, std::forward<Data>(data)), std::forward<Args>(args)...);
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
        constexpr auto is = std::make_index_sequence<decltype(hana::length(parse(s, std::forward<Data>(data))))::value>();
        return helper<Args...>()(is, parse(s, std::forward<Data>(data)), op, std::forward<Args>(args)...);
    }
};

/******************************************************************************************/

}
