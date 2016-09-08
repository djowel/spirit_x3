/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <x3/char/any_char.hpp>
#include <support/char_encoding/ascii.hpp>
#include <support/char_encoding/iso8859_1.hpp>
#include <support/char_encoding/standard.hpp>
#include <support/char_encoding/standard_wide.hpp>

namespace x3 {

    namespace standard {
        using char_type = any_char<char_encoding::standard>;
        auto const char_ = char_type{};

        inline literal_char<char_encoding::standard, unused_type>
        lit(char ch) {return { ch }; }

        inline literal_char<char_encoding::standard, unused_type> lit(wchar_t ch) {return { ch }; }
    }

    using standard::char_type;
    using standard::char_;
    using standard::lit;

    namespace standard_wide {
        typedef any_char<char_encoding::standard_wide> char_type;
        auto const char_ = char_type{};

        inline literal_char<char_encoding::standard_wide, unused_type>
        lit(wchar_t ch) {return { ch }; }
    }

    namespace ascii {
        typedef any_char<char_encoding::ascii> char_type;
        auto const char_ = char_type{};

        inline literal_char<char_encoding::ascii, unused_type>
        lit(char ch) {return { ch }; }

        inline literal_char<char_encoding::ascii, unused_type>
        lit(wchar_t ch) {return { ch }; }
    }

    namespace iso8859_1 {
        typedef any_char<char_encoding::iso8859_1> char_type;
        auto const char_ = char_type{};

        inline literal_char<char_encoding::iso8859_1, unused_type> lit(char ch) {return { ch }; }

        inline literal_char<char_encoding::iso8859_1, unused_type> lit(wchar_t ch) {return { ch }; }
    }

    namespace extension {
        template <> struct as_parser<char> {
            typedef literal_char< char_encoding::standard, unused_type> type;
            typedef type value_type;
            static type call(char ch) {return { ch }; }
        };

        template <> struct as_parser<wchar_t> {
            typedef literal_char< char_encoding::standard_wide, unused_type> type;
            typedef type value_type;
            static type call(wchar_t ch) {return { ch }; }
        };

        template <> struct as_parser<char [2]> {
            typedef literal_char< char_encoding::standard, unused_type> type;
            typedef type value_type;
            static type call(char const ch[]) {return { ch[0] }; }
        };

        template <> struct as_parser<wchar_t [2]> {
            typedef literal_char< char_encoding::standard_wide, unused_type> type;
            typedef type value_type;
            static type call(wchar_t const ch[] ) {return { ch[0] }; }
        };
    }

}
