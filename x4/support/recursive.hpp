#pragma once
#include <boost/checked_delete.hpp>
#include <boost/variant/recursive_wrapper.hpp>

namespace x4 {

/******************************************************************************************/

template <class T>
class recursive_wrap {
    struct deleter {void operator()(T *t) const {boost::checked_delete(t);}};
    std::unique_ptr<T, deleter> ptr;

public:
    using value_type = T;

    auto & value() {return ptr->value();}
    auto const & value() const {return ptr->value();}

    template <class ...Ts>
    explicit recursive_wrap(Ts &&...ts) : ptr(new T{std::forward<Ts>(ts)...}) {}

    recursive_wrap(recursive_wrap const &other) : ptr(new T{other.value()}) {}
    recursive_wrap(recursive_wrap &&) = default;

    recursive_wrap & operator=(recursive_wrap w) {swap(w); return *this;}

    void swap(recursive_wrap &other) {ptr.swap(other.ptr);}
};

template <class T>
void swap(recursive_wrap<T> &r1, recursive_wrap<T> &r2) {r1.swap(r2);}

/******************************************************************************************/

template <class T>
class nonrecursive_wrap {
    T m_value;

public:
    using value_type = T;

    template <class ...Ts>
    explicit nonrecursive_wrap(Ts &&...ts) : m_value(std::forward<Ts>(ts)...) {}

    auto & value() {return m_value;}
    auto const & value() const {return m_value;}

    friend void swap(nonrecursive_wrap &w1, nonrecursive_wrap &w2) {swap(w1.value, w2.value);}
};

/******************************************************************************************/

}
