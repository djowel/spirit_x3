#pragma once
#include "detail.hpp"

namespace x4 {

template <class T>
class literal : expression_base {
    T m_value;

public:
    constexpr literal(T t) : m_value(std::move(t)) {}

    constexpr T value() const {return m_value;}

    template <class Window>
    T check(Window &w) const {
        if (w && w.compare()(*w, value())) return *(w++);
        else return static_cast<T>(0);
    }

    constexpr bool success(T const &t) {return bool(t);}
    constexpr auto parse(T const &t) const {return t;}
};

constexpr auto operator""_x(char c) {return literal<char>(c);}
constexpr auto operator""_x(char16_t c) {return literal<char16_t>(c);}
constexpr auto operator""_x(char32_t c) {return literal<char32_t>(c);}
constexpr auto operator""_x(wchar_t c) {return literal<wchar_t>(c);}

template <class T, int_if<(is_character<T>)> = 0>
constexpr auto expression(T t) {return literal<T>(std::move(t));}

}
