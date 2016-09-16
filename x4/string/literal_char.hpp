#pragma once
#include "detail.hpp"

namespace x4 {

template <class T>
class literal : parser_base {
    T m_value;

public:
    constexpr literal(T t) : m_value(std::move(t)) {}

    constexpr T value() const {return m_value;}

    template <class Window>
    T operator()(check_base, Window &w) const {
        if (w && w.compare()(*w, value())) return *(w++);
        else return static_cast<T>(0);
    }

    constexpr bool operator()(valid_t, T t) const {return bool(t);}

    constexpr auto operator()(parse_base, T t) const {return t;}
};

namespace literals {
    constexpr auto operator""_x(char c) {return literal<char>(c);}
    constexpr auto operator""_x(char16_t c) {return literal<char16_t>(c);}
    constexpr auto operator""_x(char32_t c) {return literal<char32_t>(c);}
    constexpr auto operator""_x(wchar_t c) {return literal<wchar_t>(c);}
}

template <class T>
struct expr_t<T, void_if<(is_character<T>)>> {
    constexpr auto operator()(T t) const {return literal<T>(std::move(t));}
};

}
