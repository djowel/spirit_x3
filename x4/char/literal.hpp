#pragma once

namespace x4 {

template <class T>
struct literal {
    T t;

    template <class Window>
    Window check(Window w) {
        if (*w == t) w.fail();
        else ++w;
        return window;
    }
};

}
