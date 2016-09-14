#pragma once
#include "detail.hpp"
#include "../parse/common.hpp"

namespace x4 {

/******************************************************************************************/

template <class String>
class literal_string : parser_base {

    using string = std::conditional_t<std::is_pointer<String>::value,
        detail::string_wrap<std::remove_cv_t<std::remove_pointer_t<String>>>,
        std::conditional_t<std::is_array<String>::value,
            detail::array_wrap<std::remove_cv_t<std::remove_extent_t<String>>, std::extent<String>::value>,
            String
        >
    >;

    string str;

public:

    using char_type = std::decay_t<decltype(*std::begin(str))>;

    constexpr literal_string(string s) : str(std::move(s)) {}

    // Return pair(iterator, bool) where bool is the success and iterator is the start of the string
    template <class Window>
    auto check(Window &w) const {
        return w.no_skip([&](auto &w) {
            auto its = std::mismatch(std::begin(str), std::end(str), w.begin(), w.end(), w.compare());
            auto data = std::make_pair(w.begin(), std::move(its.first) == std::end(str));
            if (data.second) w.set_begin(std::move(its.second));
            return data;
        });
    }

    template <class Data>
    constexpr auto success(Data const &data) const {return data.second;}

    template <class Data>
    constexpr auto parse(Data const &data) const {
        return container_type<char_type>(data.first, std::next(data.first, detail::string_length(str)));
    }
};

/******************************************************************************************/

namespace literals {
    constexpr auto operator""_x(char const *c, std::size_t n) {return literal_string<char const *>(detail::string_wrap<char>(c, n));}
    constexpr auto operator""_x(char16_t const *c, std::size_t n) {return literal_string<char16_t const *>(detail::string_wrap<char16_t>(c, n));}
    constexpr auto operator""_x(char32_t const *c, std::size_t n) {return literal_string<char32_t const *>(detail::string_wrap<char32_t>(c, n));}
    constexpr auto operator""_x(wchar_t const *c, std::size_t n) {return literal_string<wchar_t const *>(detail::string_wrap<wchar_t>(c, n));}
}

/******************************************************************************************/

template <class T>
struct expr_t<T, void_if<(std::is_pointer<T>::value && is_character<std::remove_cv_t<std::remove_pointer_t<T>>>)>> {
    using char_type = std::remove_cv_t<std::remove_pointer_t<T>>;
    constexpr auto operator()(char_type const *t) const {
        auto i = t;
        for (; *i != static_cast<char_type>(0); ++i) {}
        return literal_string<char_type const *>(detail::string_wrap<char_type>(t, i - t));
    }
};

template <class T, std::size_t N>
struct expr_t<T[N], void_if<(is_character<std::remove_cv_t<T>>)>> {
    constexpr auto operator()(T const (&t)[N]) const {return literal_string<T const[N]>(detail::array_wrap<T, N>(t));}
};

/******************************************************************************************/

template <class T, class=void> struct is_string_t : std::false_type {};
template <class ...Ts> struct is_string_t<std::basic_string<Ts...>, void> : std::true_type {};
template <class T> static constexpr auto is_string = hana::bool_c<is_string_t<T>::value>;

template <class T>
struct expr_t<T, void_if<(is_string<T>)>> {
    constexpr auto operator()(T t) const {return literal_string<T>(std::move(t));}
};

/******************************************************************************************/

}

/*

namespace standard {
    inline literal_string<char const*, char_encoding::standard>
    string(char const *s) {return {s};}

    inline literal_string<std::basic_string<char>, char_encoding::standard>
    string(std::basic_string<char> const &s) {return { s };}

    inline literal_string<char const*, char_encoding::standard, unused_type>
    lit(char const *s) {return { s };}

    template <typename Char>
    literal_string<std::basic_string<Char>, char_encoding::standard, unused_type>
    lit(std::basic_string<Char> const &s) {return { s };}
}

namespace standard_wide {
    inline literal_string<wchar_t const*, char_encoding::standard_wide>
    string(wchar_t const *s) {return {s};}

    inline literal_string<std::basic_string<wchar_t>, char_encoding::standard_wide>
    string(std::basic_string<wchar_t> const &s) {return {s};}

    inline literal_string<wchar_t const*, char_encoding::standard_wide, unused_type>
    lit(wchar_t const *s) {return {s};}

    inline literal_string<std::basic_string<wchar_t>, char_encoding::standard_wide, unused_type>
    lit(std::basic_string<wchar_t> const &s) {return { s };}
}

namespace ascii {
    inline literal_string<wchar_t const*, char_encoding::ascii>
    string(wchar_t const *s) {return {s};}

    inline literal_string<std::basic_string<wchar_t>, char_encoding::ascii>
    string(std::basic_string<wchar_t> const &s) {return {s};}

    inline literal_string<char const*, char_encoding::ascii, unused_type>
    lit(char const *s) {return {s};}

    template <typename Char>
    literal_string<std::basic_string<Char>, char_encoding::ascii, unused_type>
    lit(std::basic_string<Char> const &s) {return {s};}
}

namespace iso8859_1 {
    inline literal_string<wchar_t const*, char_encoding::iso8859_1>
    string(wchar_t const *s) {return {s};}

    inline literal_string<std::basic_string<wchar_t>, char_encoding::iso8859_1>
    string(std::basic_string<wchar_t> const &s) {return {s};}

    inline literal_string<char const*, char_encoding::iso8859_1, unused_type>
    lit(char const *s) {return { s };}

    template <typename Char>
    literal_string<std::basic_string<Char>, char_encoding::iso8859_1, unused_type>
    lit(std::basic_string<Char> const &s) {return {s};}
}

using standard::string;
using standard::lit;
using standard_wide::string;
using standard_wide::lit;

namespace extension {
    template <int N>
    struct as_parser<char[N]> {
        typedef literal_string<char const*, char_encoding::standard, unused_type> type;
        typedef type value_type;

        static type call(char const *s) {return type(s);}
    };

    template <int N>
    struct as_parser<char const[N]> : as_parser<char[N]> {};

    template <int N>
    struct as_parser<wchar_t[N]> {
        typedef literal_string< wchar_t const*, char_encoding::standard_wide, unused_type> type;
        typedef type value_type;

        static type call(wchar_t const *s) {return type(s); }
    };

    template <int N>
    struct as_parser<wchar_t const[N]> : as_parser<wchar_t[N]> {};

    template <>
    struct as_parser<char const*> {
        typedef literal_string<char const*, char_encoding::standard, unused_type> type;
        typedef type value_type;

        static type call(char const *s) {return type(s); }
    };

    template <typename Char>
    struct as_parser< std::basic_string<Char> > {
        typedef literal_string< Char const*, char_encoding::standard, unused_type> type;
        typedef type value_type;

        static type call(std::basic_string<Char> const &s) {return type(s.c_str());}
    };
}
}
*/
/******************************************************************************************/

