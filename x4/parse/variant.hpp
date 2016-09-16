#pragma once
#include "common.hpp"

#include <boost/hana/prepend.hpp>
#include <boost/hana/plus.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/minus.hpp>
#include <boost/variant.hpp>

#include "../support/variant.hpp"
#include <utility>

namespace x4 {

/******************************************************************************************/

template <class Index, class T>
class index_wrapper {
    T value;
public:
    template <class ...Args, int_if<std::is_constructible<T, Args &&...>::value> = 0>
    constexpr explicit index_wrapper(Args &&...args) : value{std::forward<Args>(args)...} {}

    using value_type = T;

    T & operator*() {return value;}
    T const & operator*() const {return value;}
};

/******************************************************************************************/

template <class T>
struct is_complete_helper {
    template <class U>
    static auto test(U*)  -> std::integral_constant<bool, sizeof(U) == sizeof(U)>;
    static auto test(...) -> std::false_type;
    using type = decltype(test((T*) nullptr));
};

template <class T>
static constexpr auto is_complete = false;// is_complete_helper<T>::type::value;

/******************************************************************************************/

static constexpr auto index_wrap = hana::fuse(hana::template_<index_wrapper>);

template <class I, class T> struct recursive_t {
    using type = std::conditional_t<
        is_complete<T>,
        index_wrapper<I, T>,
        boost::recursive_wrapper<index_wrapper<I, T>>
    >;
};

static constexpr auto recursive_c = hana::fuse(hana::metafunction<recursive_t>);

/******************************************************************************************/

/*
template <class ...Types>
class variant {
public:

    static constexpr auto types = hana::tuple_t<Types...>;

protected:
    static constexpr auto items = hana::transform(enumerate(types), index_wrap);
    static constexpr auto rtypes = hana::transform(enumerate(types), recursive_c);

    template <class I, class F, int_if<I::value + 1 == sizeof...(Types)> = 0>
    constexpr auto visit_(I i, std::size_t n, F &&f) const {return std::forward<F>(f)(i);}

    template <class I, class F, int_if<I::value + 1 < sizeof...(Types)> = 0>
    constexpr auto visit_(I i, std::size_t n, F &&f) const {
        if (i() == n) return std::forward<F>(f)(i);
        else return visit_(i + 1_c, n, std::forward<F>(f));
    }

public:
    using data_type = decltype(*hana::unpack(rtypes, hana::template_<boost::variant>));

    data_type data;

    template <class I, class ...Ts>
    explicit variant(I i, Ts &&...ts) : data(decltype(*items[i])(std::forward<Ts>(ts)...)) {}

    auto which() const {return data.which();}

    template <class I>
    auto & operator[](I i) {return *boost::get<decltype(*items[i])>(data);}

    template <class I>
    auto const & operator[](I i) const {return *boost::get<decltype(*items[i])>(data);}

    template <class I, class ...Ts>
    void emplace(I i, Ts &&...ts) {data = decltype(*items[i])(std::forward<Ts>(ts)...);}

    template <class F> constexpr auto visit(F &&f) const {
        return visit_(0_c, which(), [&](auto i) {return std::forward<F>(f)(i, (*this)[i]);});
    }

    template <class F> constexpr auto visit(F &&f) {
        return visit_(0_c, which(), [&](auto i) {return std::forward<F>(f)(i, (*this)[i]);});
    }
};
*/
/******************************************************************************************/

static constexpr auto variant_c = hana::fuse(hana::template_<variant>);

/******************************************************************************************/

template <class ...Types>
class optional_variant : public variant<decltype(*nothing_c), Types...> {
    using base = variant<decltype(*nothing_c), Types...>;
public:
    optional_variant() : base(0_c) {}

    auto which() const {return base::which() - 1_c;}
    operator bool() const {return base::which();}

    template <class I> decltype(auto) operator[](I i) {return base::operator[](i + 1_c);}
    template <class I> decltype(auto) operator[](I i) const {return base::operator[](i + 1_c);}

    template <class I, class ...Ts>
    void emplace(I i, Ts &&...ts) {base::emplace(i + 1_c, std::forward<Ts>(ts)...);}

    template <class F> constexpr auto visit(F &&f) const {
        return base::visit([&](auto i, auto const &t) {return std::forward<F>(f)(i-1_c, t);}, 1_c);
    }

    template <class F> constexpr auto visit(F &&f) {
        return base::visit([&](auto i, auto &t) {return std::forward<F>(f)(i-1_c, t);}, 1_c);
    }

    ~optional_variant() {} // {std::cout << "optional destroy " << ((void*)this)  << std::endl;}
};

static constexpr auto optional_variant_c = hana::fuse(hana::template_<optional_variant>);

/******************************************************************************************/

}
