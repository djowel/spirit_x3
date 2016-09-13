#pragma once
#include "common.hpp"

#include <boost/hana/prepend.hpp>
#include <boost/hana/plus.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/minus.hpp>

#include <boost/variant.hpp>
#include <utility>

namespace x4 {

/******************************************************************************************/

template <class Index, class T>
struct index_wrapper {
    template <class ...Args> index_wrapper(Args &&...args) : value(std::forward<Args>(args)...) {}
    T value;
};

static constexpr auto index_wrap = hana::fuse(hana::template_<index_wrapper>);

template <class T, class=void> struct is_recursive : std::true_type {};

template <class T> struct recursive_t {
    using type = std::conditional_t<!is_recursive<T>::value || std::is_empty<T>::value || std::is_pod<T>::value, T, boost::recursive_wrapper<T>>;
};
static constexpr auto recursive_c = hana::metafunction<recursive_t>;

/******************************************************************************************/

template <class ...Types>
class variant : expression_base {
protected:
    static constexpr auto items = hana::transform(enumerate(hana::tuple_t<Types...>), index_wrap);

    template <class I, class F, int_if<I::value + 1 == sizeof...(Types)> = 0>
    constexpr auto visit_(I i, std::size_t n, F &&f) const {return std::forward<F>(f)(i);}

    template <class I, class F, int_if<I::value + 1 < sizeof...(Types)> = 0>
    constexpr auto visit_(I i, std::size_t n, F &&f) const {
        if (i() == n) return std::forward<F>(f)(i);
        else return visit_(i + 1_c, n, std::forward<F>(f));
    }

public:
    using data_type = decltype(*hana::unpack(hana::transform(items, recursive_c), hana::template_<boost::variant>));

    data_type data;

    template <class I, class ...Ts>
    explicit variant(I i, Ts &&...ts) : data(decltype(*items[i])(std::forward<Ts>(ts)...)) {}

    auto which() const {return data.which();}

    template <class I>
    auto & operator[](I i) {return boost::get<decltype(*items[i])>(data).value;}

    template <class I>
    auto const & operator[](I i) const {return boost::get<decltype(*items[i])>(data).value;}

    template <class I, class ...Ts>
    void emplace(I i, Ts &&...ts) {data = decltype(*items[i])(std::forward<Ts>(ts)...);}

    template <class F> constexpr auto visit(F &&f) const {
        return visit_(0_c, which(), [&](auto i) {return std::forward<F>(f)(i, (*this)[i]);});
    }

    template <class F> constexpr auto visit(F &&f) {
        return visit_(0_c, which(), [&](auto i) {return std::forward<F>(f)(i, (*this)[i]);});
    }
};

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
        return base::visit_(1_c, base::which(), [&](auto i) {return std::forward<F>(f)(i-1_c, (*this)[i-1_c]);});
    }

    template <class F> constexpr auto visit(F &&f) {
        return base::visit_(1_c, base::which(), [&](auto i) {return std::forward<F>(f)(i-1_c, (*this)[i-1_c]);});
    }

    void reset() {base::emplace(0_c, hana::nothing);}

    template <class T, bool B=true, int_if<B && sizeof...(Types) == 1> = 0>
    auto & operator=(T &&t) {emplace(0_c, std::forward<T>(t)); return *this;}

    template <bool B=true, int_if<B && sizeof...(Types) == 1> = 0>
    auto & operator*() {return (*this)[0_c];}

    template <bool B=true, int_if<B && sizeof...(Types) == 1> = 0>
    auto const & operator*() const {return (*this)[0_c];}
};

static constexpr auto optional_variant_c = hana::fuse(hana::template_<optional_variant>);

/******************************************************************************************/

template <class T> using optional_type = optional_variant<T>;
static constexpr auto optional_c = hana::template_<optional_variant>;

/******************************************************************************************/

}
