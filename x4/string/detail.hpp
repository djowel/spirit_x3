#pragma once

namespace x4 { namespace detail {

/******************************************************************************************/

template <class T, std::size_t N>
constexpr auto string_length(T[N]) {return hana::size_c<N>;}

template <class T>
constexpr auto string_length(T const &t) -> decltype(t.size()) {return t.size();}

/******************************************************************************************/

template <class T, class=void>
struct is_character_t : public std::false_type {};

template <> struct is_character_t<char, void> : public std::true_type {};
template <> struct is_character_t<char16_t, void> : public std::true_type {};
template <> struct is_character_t<char32_t, void> : public std::true_type {};
template <> struct is_character_t<wchar_t, void> : public std::true_type {};

/******************************************************************************************/

template <class T>
class string_wrap {
    T const * const data;
    std::size_t const m_size;
public:
    template<std::size_t N> constexpr string_wrap(const T(&t)[N]) : data(t), m_size(N-1) {}
    constexpr string_wrap(T const *t, std::size_t n) : data(t), m_size(n) {}

    constexpr auto begin() const {return data;}
    constexpr auto end() const {return data + m_size;}
    constexpr auto size() const {return m_size;}
};

/******************************************************************************************/

}

template <class T> static constexpr auto is_character = hana::bool_c<detail::is_character_t<T>::value>;

template <class T>
struct container_type_t<T, void_if<(is_character<T>)>> {using type = std::basic_string<T>;};

}
