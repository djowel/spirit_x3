#pragma once
#include "uint.hpp"

namespace xf {

/******************************************************************************************/

template <class T, unsigned Radix = 10>
struct int_parser : parser_base {
    template <class Window>
    auto operator()(check_base, Window &w) const {
        return w.no_skip([](auto &w) {
            bool positive = true;
            if (w) {
                if (w && *w == '-') {++w; positive = false;}
                else if (w && *w == '+') ++w;
            }
            return std::make_pair(digit_range(w, hana::size_c<std::numeric_limits<T>::digits10 - 2>), positive);
        });
    }

    template <class Data>
    bool operator()(valid_t, Data const &data) const {return data.first[2_c];}

    template <class Data>
    T operator()(parse_base, Data data) const {
        auto const n = std::distance(data.first[0_c], data.first[1_c]);
        auto t = std::inner_product(data.first[0_c], data.first[1_c], mask<T, Radix>.end() - n, static_cast<T>(0)) - remainder<T, Radix>[n];
        return data.second ? t : -t;
    }
};

/******************************************************************************************/

#define BOOST_SPIRIT_X3_INT_PARSER(int_type, name)                              \
    typedef int_parser<int_type> name##type;                                    \
    name##type const name = {};                                                 \
    /***/

    BOOST_SPIRIT_X3_INT_PARSER(long, long_x)
    BOOST_SPIRIT_X3_INT_PARSER(short, short_x)
    BOOST_SPIRIT_X3_INT_PARSER(int, int_x)
    BOOST_SPIRIT_X3_INT_PARSER(long long, llong_x)
    BOOST_SPIRIT_X3_INT_PARSER(std::ptrdiff_t, ptrdiff_x)

    BOOST_SPIRIT_X3_INT_PARSER(int8_t, int8_x)
    BOOST_SPIRIT_X3_INT_PARSER(int16_t, int16_x)
    BOOST_SPIRIT_X3_INT_PARSER(int32_t, int32_x)
    BOOST_SPIRIT_X3_INT_PARSER(int64_t, int64_x)

#undef BOOST_SPIRIT_X3_INT_PARSER

/******************************************************************************************/

}
