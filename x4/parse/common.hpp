#pragma once

#include <boost/optional.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/zip.hpp>
#include <boost/hana/optional.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/fuse.hpp>
#include <boost/hana/or.hpp>
#include <boost/hana/functional/always.hpp>
#include <boost/hana/functional/overload_linearly.hpp>

#include <vector>
#include <iostream>

namespace x4 {

using namespace boost::hana::literals;
namespace hana = boost::hana;

/******************************************************************************************/

template <bool B> using int_if = std::enable_if_t<B, int>;
template <bool B> using void_if = std::enable_if_t<B, void>;
template <class T> T & declref();

namespace detail {template<class ...Ts> struct make_void {using type = void;};}
template<class ...Ts> using void_t = typename detail::make_void<Ts...>::type;

/******************************************************************************************/

struct no_void_t {
    constexpr auto operator[](no_void_t) const {return hana::nothing;}
    template <class T>
    constexpr T && operator[](T &&t) const {return std::forward<T>(t);}
    template <class T>
    constexpr friend T && operator,(T &&t, no_void_t) {return std::forward<T>(t);}
};

static constexpr auto no_void = no_void_t();

/******************************************************************************************/

static constexpr auto always_true = hana::always(hana::true_c);
static constexpr auto always_false = hana::always(hana::false_c);

template <class T> T operator*(hana::basic_type<T> t);
static constexpr auto nothing_c = hana::type_c<std::decay_t<decltype(hana::nothing)>>;

template <std::size_t N>
static constexpr auto indices_c = hana::to_tuple(hana::range_c<long long, 0, N>);

template <class T>
constexpr auto enumerate(T &&t) {
    return hana::zip(indices_c<decltype(hana::length(t))::value>, std::forward<T>(t));
}

/******************************************************************************************/

template <class T> using optional_type = boost::optional<T>;
static constexpr auto optional_c = hana::template_<boost::optional>;

static constexpr auto tuple_c = hana::fuse(hana::template_<hana::tuple>);
static constexpr auto container_c = hana::template_<std::vector>;

/******************************************************************************************/

struct expression_base {};
template <class T, class=void> struct is_expression_t : std::false_type {};
template <class T> struct is_expression_t<T, std::enable_if_t<std::is_base_of<expression_base, T>::value>> : std::true_type {};

template <class T> static constexpr auto is_expression = hana::bool_c<is_expression_t<T>::value>;

/******************************************************************************************/

struct parser_base : expression_base {constexpr parser_base() {}};
template <class T, class=void> struct is_parser_t : std::false_type {};
template <class T> struct is_parser_t<T, std::enable_if_t<std::is_base_of<parser_base, T>::value>> : std::true_type {};

template <class T> static constexpr auto is_parser = hana::bool_c<is_parser_t<T>::value>;

/******************************************************************************************/

template <class T, class=void>
struct expr_t;

template <class T>
struct expr_t<T, void_if<is_expression<T>>> {
    template <class T_>
    constexpr T_ && operator()(T_ &&t) const {return std::forward<T_>(t);}
};

template <class T> constexpr decltype(auto) expr(T &&t) {
    return expr_t<std::remove_cv_t<std::remove_reference_t<T>>>()(std::forward<T>(t));
}

/******************************************************************************************/

static constexpr auto decay = hana::metafunction<std::decay>;

template <class T>
constexpr auto types_of(T const &t) {return hana::transform(hana::transform(t, hana::decltype_), decay);}

template <class T>
constexpr auto type_of(T const &t) {return decay(hana::decltype_(t));}

/******************************************************************************************/

template <class T, class=void>
struct container_type_t {using type = std::vector<T>;};

template <class T> using container_type = typename container_type_t<T>::type;

template <class T, class ...Args>
void append(T &container, Args &&...args) {
    hana::overload_linearly(
        [](auto &t, auto &&...ts) -> decltype(t.emplace_back(std::forward<decltype(ts)>(ts)...)) {
            return t.emplace_back(std::forward<decltype(ts)>(ts)...);
        },
        [](auto &t, auto &&...ts) -> decltype(t.push_back(typename T::value_type(std::forward<decltype(ts)>(ts)...))) {
            return t.push_back(typename T::value_type(std::forward<decltype(ts)>(ts)...));
        }
    )(container, std::forward<Args>(args)...);
}

/******************************************************************************************/

}
