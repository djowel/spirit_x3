#pragma once

#include <boost/optional.hpp>
#include <boost/blank.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/zip.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/fuse.hpp>
#include <boost/hana/or.hpp>

#include <vector>

namespace x4 {

using namespace boost::hana::literals;
namespace hana = boost::hana;

/******************************************************************************************/

template <class T> T operator*(hana::basic_type<T> t);
static constexpr auto blank_c = hana::type_c<boost::blank>;

template <std::size_t N>
static constexpr auto indices_c = hana::to_tuple(hana::range_c<long long, 0, N>);

template <class T>
constexpr auto enumerate(T &&t) {
    return hana::zip(indices_c<decltype(hana::length(t))::value>, std::forward<T>(t));
}

/******************************************************************************************/

static constexpr auto optional_c = hana::template_<boost::optional>;
static constexpr auto tuple_c = hana::fuse(hana::template_<hana::tuple>);
static constexpr auto container_c = hana::template_<std::vector>;

/******************************************************************************************/

struct expression_base {};

template <class T, class=void>
struct is_expression_t : std::false_type {};

template <class T>
struct is_expression_t<T, std::enable_if_t<std::is_base_of<expression_base, T>::value>> : std::true_type {};

template <class T> static constexpr auto is_expression = hana::bool_c<is_expression_t<T>::value>;

/******************************************************************************************/

template <bool B> using int_if = std::enable_if_t<B, int>;
template <bool B> using void_if = std::enable_if_t<B, void>;

/******************************************************************************************/

}
