#pragma once
#include "../parse/common.hpp"
#include <boost/hana/power.hpp>
#include <boost/hana/plus.hpp>
#include <boost/hana/greater.hpp>
#include <boost/hana/scan_left.hpp>
#include <boost/hana/reverse.hpp>
#include <boost/hana/append.hpp>
#include <cstdint>
#include <numeric>
#include <array>

namespace x4 {

/******************************************************************************************/

template <class T, class From, std::size_t ...Is>
constexpr auto make_array(From const &f, std::index_sequence<Is...>) {
    return std::array<T, sizeof...(Is)>{hana::at_c<Is>(f)...};
}

/******************************************************************************************/

template <class T, class R>
constexpr auto radix_mask(R radix) {
    constexpr auto is = hana::reverse(indices_c<std::numeric_limits<T>::digits10 - 2>);
    constexpr auto ts = hana::transform(is, hana::partial(hana::power, radix));
    return make_array<T>(ts, std::make_index_sequence<decltype(hana::length(ts))::value>());
}

/******************************************************************************************/

template <class R, class S>
struct radix_increment {
    R radix;
    S start;
    template <class I, class T>
    constexpr auto operator()(T t, I) const {return t * radix + start;}
};

template <class T, class R, class S>
constexpr auto radix_remainder(R radix, S start) {
    constexpr auto is = indices_c<std::numeric_limits<T>::digits10 - 2>;
    constexpr auto ts = hana::scan_left(is, hana::integral_constant<T, 0>(), radix_increment<R, S>{radix, start});
    return make_array<T>(ts, std::make_index_sequence<decltype(hana::length(ts))::value>());
}

/******************************************************************************************/

template <class T, unsigned Radix>
static constexpr auto remainder = radix_remainder<T>(hana::integral_constant<T, Radix>(), hana::integral_constant<T, '0'>());
template <class T, unsigned Radix>
static constexpr auto mask = radix_mask<T>(hana::integral_constant<T, Radix>());

/******************************************************************************************/

template <class Window, class N>
auto digit_range(Window &w, N n) {
    auto r = hana::make_tuple(w.begin(), w.begin(), w && std::isdigit(*w));
    if (!r[2_c]) return r;

    while (w && *w == '0') ++w;
    unsigned digits = 0;
    r[0_c] = w.begin(); // set start of nonzeros
    while (w && std::isdigit(*w)) {++w; ++digits;}

    if (digits > n) { // revert window and return false
        r[2_c] = false;
        w.set_begin(r[1_c]);
    } else r[1_c] = w.begin(); // set end of nonzeros
    return r;
}

/******************************************************************************************/

template <class T, unsigned Radix = 10>
struct uint_parser : parser_base {
    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    auto operator()(Tag, Window &w) const {
        return w.no_skip([](auto &w) {
            return digit_range(w, hana::size_c<std::numeric_limits<T>::digits10>);
        });
    }

    template <class Data>
    bool operator()(valid_t, Data const &data) const {return data[2_c];}

    template <class Tag, class Data, int_if<is_parse<Tag>> = 0>
    T operator()(Tag, Data data) const {
        auto const n = std::distance(data[0_c], data[1_c]);
        return std::inner_product(data[0_c], data[1_c], mask<T, Radix>.end() - n, static_cast<T>(0)) - remainder<T, Radix>[n];
    }
};

/******************************************************************************************/

#define BOOST_SPIRIT_X3_UINT_PARSER(uint_type, name)                            \
    typedef uint_parser<uint_type> name##type;                                  \
    name##type const name = {};                                                 \
    /***/

    BOOST_SPIRIT_X3_UINT_PARSER(unsigned long, ulong_x)
    BOOST_SPIRIT_X3_UINT_PARSER(unsigned short, ushort_x)
    BOOST_SPIRIT_X3_UINT_PARSER(unsigned int, uint_x)
    BOOST_SPIRIT_X3_UINT_PARSER(unsigned long long, ullong_x)

    BOOST_SPIRIT_X3_UINT_PARSER(uint8_t, uint8_x)
    BOOST_SPIRIT_X3_UINT_PARSER(uint16_t, uint16_x)
    BOOST_SPIRIT_X3_UINT_PARSER(uint32_t, uint32_x)
    BOOST_SPIRIT_X3_UINT_PARSER(uint64_t, uint64_x)

    BOOST_SPIRIT_X3_UINT_PARSER(std::size_t, size_x)

#undef BOOST_SPIRIT_X3_UINT_PARSER

}
