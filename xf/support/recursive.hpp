#pragma once
#include <boost/checked_delete.hpp>
#include <boost/variant/recursive_wrapper.hpp>

namespace xf {

/******************************************************************************************/

template <class Type>
class recursive_wrap {
    struct deleter {void operator()(Type *t) const {boost::checked_delete(t);}};
    std::unique_ptr<Type, deleter> ptr;

public:
    using value_type = Type;

    auto & value() {return ptr->value();}
    auto const & value() const {return ptr->value();}

    template <class ...Ts>
    explicit recursive_wrap(Ts &&...ts) : ptr(new Type{std::forward<Ts>(ts)...}) {}

    template <class T> explicit operator T() const & {return *ptr;}
    template <class T> explicit operator T() && {return std::move(*ptr);}
    template <class T> explicit operator T() & {return *ptr;}

    recursive_wrap(recursive_wrap const &other) : ptr(new Type{other.value()}) {}
    recursive_wrap(recursive_wrap &&) = default;

    recursive_wrap & operator=(recursive_wrap w) {swap(w); return *this;}

    void swap(recursive_wrap &other) {ptr.swap(other.ptr);}
};

template <class Type>
void swap(recursive_wrap<Type> &r1, recursive_wrap<Type> &r2) {r1.swap(r2);}

/******************************************************************************************/

template <class Type>
class nonrecursive_wrap {
    Type m_value;

public:
    using value_type = Type;

    template <class ...Ts>
    explicit nonrecursive_wrap(Ts &&...ts) : m_value(std::forward<Ts>(ts)...) {}

    auto & value() {return m_value;}
    auto const & value() const {return m_value;}

    template <class T, int_if<std::is_constructible<T, Type const &>::value> = 0>
    explicit operator T() const & {return T(m_value);}

    template <class T, int_if<std::is_constructible<T, Type &&>::value> = 0>
    explicit operator T() && {return T(m_value);}

    template <class T, int_if<std::is_constructible<T, Type &>::value> = 0>
    explicit operator T() & {return T(m_value);}

    friend void swap(nonrecursive_wrap &w1, nonrecursive_wrap &w2) {swap(w1.value, w2.value);}
};

/******************************************************************************************/

}
