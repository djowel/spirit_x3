/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_X3_UC_TYPES_NOVEMBER_23_2008_0840PM)
#define BOOST_SPIRIT_X3_UC_TYPES_NOVEMBER_23_2008_0840PM

#if defined(_MSC_VER)
#pragma once
#endif

#include <cstdint>
#include <algorithm>
#include <type_traits>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <string>

namespace boost { namespace spirit { namespace x3
{
    using utf8_string = std::string;
    using utf32_string = std::u32string;

    template <typename Char>
    inline utf8_string to_utf8(Char value)
    {
        utf8_string result;
        using insert_iter = std::back_insert_iterator<utf8_string>;
        utf8_output_iterator<insert_iter> utf8_iter{insert_iter(result)};
        using UChar = typename std::make_unsigned<Char>::type;
        *utf8_iter = (UChar)value;
        return result;
    }

    template <typename Char>
    inline utf8_string to_utf8(Char const* str)
    {
        utf8_string result;
        using insert_iter = std::back_insert_iterator<utf8_string>;
        utf8_output_iterator<insert_iter> utf8_iter{insert_iter(result)};
        using UChar = typename make_unsigned<Char>::type;
        while (*str)
            *utf8_iter++ = (UChar)*str++;
        return result;
    }

    template <typename Char, typename Traits, typename Allocator>
    inline utf8_string
    to_utf8(std::basic_string<Char, Traits, Allocator> const& str)
    {
        utf8_string result;
        result.reserve(str.size()); // save a few mallocs
        using insert_iter = std::back_insert_iterator<utf8_string>;
        utf8_output_iterator<insert_iter> utf8_iter{insert_iter(result)};
        using UChar = typename make_unsigned<Char>::type;
        for (UChar ch : str)
            *utf8_iter++ = ch;
        return result;
    }
}}}

#endif
