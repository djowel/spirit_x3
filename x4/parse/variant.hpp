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
class index_wrapper {
    T value;
public:
    template <class ...Args, int_if<std::is_constructible<T, Args &&...>::value> = 0>
    constexpr explicit index_wrapper(Args &&...args) : value{std::forward<Args>(args)...} {}

    T & operator*() {return value;}
    T const & operator*() const {return value;}
};

/******************************************************************************************/

template <class T>
struct is_complete_helper {
    template <class U>
    static auto test(U*)  -> std::integral_constant<bool, sizeof(U) == sizeof(U)>;
    static auto test(...) -> std::false_type;
    using type = decltype(test((T*)0));
};

template <class T>
static constexpr auto is_complete = hana::false_c;//hana::bool_c<is_complete_helper<T>::type::value>;

static constexpr auto index_wrap = hana::fuse(hana::template_<index_wrapper>);

template <class T, class=void> struct is_recursive : std::true_type {};

template <class T> struct recursive_t {
    using type = std::conditional_t<!is_recursive<T>::value || is_complete<T>, T, boost::recursive_wrapper<T>>;
};

// ||

static constexpr auto recursive_c = hana::metafunction<recursive_t>;

/******************************************************************************************/

template <class ...Types>
class variant {
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
};

static constexpr auto optional_variant_c = hana::fuse(hana::template_<optional_variant>);

/******************************************************************************************/

}
