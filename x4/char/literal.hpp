#pragma once

namespace x4 {

template <class T>
struct literal : expression_base {
    T t;

    constexpr literal(T t_) : t(t_) {}

    template <class Window>
    bool check(Window &w) const {
        if (w && *w == t) {++w; return true;}
        else return false;
    }

    constexpr auto parse(T const &) const {return t;}
};

constexpr auto operator""_x(char c) {return literal<char>(c);}

//template <class T> constexpr auto lit(T t) {return literal<T>{t};}


}
