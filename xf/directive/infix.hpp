#pragma once
#include "../operator/transform.hpp"

namespace xf {

struct infix_base {};

template <class Op>
class infix : infix_base {
    template <class L>
    struct left {
        L value;
        template <class R>
        friend auto operator%(left &&l, R &&r) {return Op()(std::move(l.value), std::forward<R>(r));}
    };

    template <class R>
    struct right {
        R value;
        template <class L>
        friend auto operator%(right &&r, L &&l) {return Op()(std::forward<L>(l), std::move(r.value));}
    };

public:

    template <class R>
    friend auto operator%(infix, R &&d) {return right<std::decay_t<R>>{std::forward<R>(d)};}

    template <class L>
    friend auto operator%(L &&l, infix) {return left<std::decay_t<L>>{std::forward<L>(l)};}
};

template <class T>
struct prevent_transform<T, void_if<std::is_base_of<infix_base, T>::value>> : std::true_type {};

}
