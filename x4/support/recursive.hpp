#pragma once
#include <boost/checked_delete.hpp>

namespace x4 {

/******************************************************************************************/

template <class T>
class recursive_wrap {
    struct deleter {void operator()(T *t) const {boost::checked_delete(t);}};

    std::unique_ptr<T, deleter> ptr;
public:
    static constexpr bool wraps_recursive_value = true;

    auto & value() {return *ptr;}
    auto const & value() const {return *ptr;}

    template <class ...Ts>
    explicit recursive_wrap(Ts &&...ts) : ptr(new T{std::forward<Ts>(ts)...}) {}

    //template<bool B=true, int_if<B> = 0>
    recursive_wrap(recursive_wrap const &other) : ptr(new T(*other.ptr)) {}

    //template<bool B=true, int_if<B> = 0>
    recursive_wrap &operator=(recursive_wrap const &other) {*ptr = *other.ptr; return *this;}

    void swap(recursive_wrap &other) {ptr.swap(other.ptr);}
};

template <class T>
void swap(recursive_wrap<T> &r1, recursive_wrap<T> &r2) {r1.swap(r2);}

/******************************************************************************************/

}
