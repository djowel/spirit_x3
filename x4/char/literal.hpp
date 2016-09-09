#pragma once

namespace x4 {

template <class T>
struct literal {
    T t;

    template <class Window>
    bool check(Window w) {return *w == t;}
};

}
