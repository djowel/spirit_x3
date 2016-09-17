#pragma once
#include <boost/hana/traits.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/greater_equal.hpp>

#include "recursive.hpp"

namespace x4 {

template <class ...Types>
using aligned_union_t = typename std::aligned_union<0, Types...>::type;
static constexpr auto aligned_union = hana::fuse(hana::template_<aligned_union_t>);

/******************************************************************************************/

template <class Index>
class variant_error : std::exception {
    int n;
    static std::string const msg;
public:
    explicit variant_error(int n_) : n(n_) {}
    char const * what() const noexcept override {return msg.c_str();}
};

template <class Index>
std::string const variant_error<Index>::msg = "variant error with index " + std::to_string(Index::value);

/******************************************************************************************/

template <class V, class P>
constexpr auto index_if(V const &v, P &&p) {
    constexpr auto first = hana::reverse_partial(hana::at, 0_c);
    constexpr auto second = hana::reverse_partial(hana::at, 1_c);
    return first(hana::find_if(enumerate(v), hana::compose(std::forward<P>(p), second)).value());
}

/******************************************************************************************/

template <bool Recursive, class ...Types>
class variant {
    static constexpr auto size = hana::int_c<sizeof...(Types)>;
    static constexpr auto types = hana::if_(hana::bool_c<Recursive>,
        hana::tuple_t<recursive_wrap<Types>...>,
        hana::tuple_t<nonrecursive_wrap<Types>...>
    );
    static constexpr auto storage = aligned_union(types);

    static constexpr auto can_move = hana::all_of(types, hana::traits::is_move_constructible);
    static constexpr auto can_copy = hana::all_of(types, hana::traits::is_copy_constructible);
    static constexpr auto can_swap = can_move;//hana::all_of(types, hana::traits::is_swappable);

    template <class I, class ...Ts, int_if<I::value >= 0> = 0>
    static constexpr auto can_construct(I i, Ts ...ts) {return hana::traits::is_constructible(types[i], ts...);}

    decltype(*storage) data;
    std::conditional_t<sizeof...(Types) <= 1, decltype(size), int> status;

    /**************************************************************************************/

    constexpr auto ptr() const {return static_cast<void const *>(std::addressof(data));}
    constexpr auto ptr() {return static_cast<void *>(std::addressof(data));}

    struct move {
        template <class I>
        void operator()(I i, variant &self, variant &other) const {
            new(self.ptr()) decltype(*types[i]){std::move(other[i])};
        }
    };

    struct copy {
        template <class I>
        void operator()(I i, variant &self, variant const &other) const {
            new(self.ptr()) decltype(*types[i]){other[i]};
        }
    };

    struct destroy {
        template <class I, class T=decltype(*types[I()])>
        void operator()(I i, variant &self) const {static_cast<T *>(self.ptr())->~T();}
    };

    struct at {
        template <class I, class Var, class F, class ...Ts>
        decltype(auto) operator()(I i, Var &&self, F &&f, Ts &&...ts) const {
            return std::forward<F>(f)(i, self[i], std::forward<Ts>(ts)...);
        }
    };

protected:

    // can redo to use jump table
    template <class F, class I, class N, class ...Ts, int_if<I::value + 1 >= size> = 0>
    static decltype(auto) fold(I i, N n, F &&f, Ts &&...ts) {
        static_assert(i >= 0_c && i < size, "variant out of range");
        assert(i == n); return std::forward<F>(f)(i, std::forward<Ts>(ts)...);
    }

    template <class F, class I, class N, class ...Ts, int_if<I::value + 2 <= size> = 0>
    static decltype(auto) fold(I i, N n, F &&f, Ts &&...ts) {
        static_assert(i >= 0_c && i < size, "variant out of range");
        if (n == i) return std::forward<F>(f)(i, std::forward<Ts>(ts)...);
        else return fold(i + 1_c, n, std::forward<F>(f), std::forward<Ts>(ts)...);
    }

public:

    constexpr auto index() const {return status;}

    template <bool B=true, int_if<B && hana::traits::is_default_constructible(types[0_c])> = 0>
    constexpr variant() : status(0_c) {new(ptr()) decltype(*types[0_c]){};}

    template <class I, class ...Ts, int_if<can_construct(std::declval<I>(), hana::type_c<Ts &&>...)> = 0>
    explicit variant(I i, Ts &&...ts) : status(i) {new(ptr()) decltype(*types[i])(std::forward<Ts>(ts)...);}

    /**************************************************************************************/

    template <bool B=true, int_if<B && can_move> = 0>
    variant(variant &&v) : status(v.status) {fold(0_c, status, move(), *this, v);}

//    template <bool B=true, int_if<B && can_copy> = 0>
//    variant(variant const &v) : status(v.status) {fold(0_c, status, copy(), *this, v);}
//
//    friend void swap(variant &v1, variant &v2) {std::swap(v1.data, v2.data); std::swap(v1.status, v2.status);}
//
//    template <bool B=true, int_if<B && (can_move || can_copy)> = 0>
//    variant & operator=(variant other) {swap(*this, other); return *this;}

    /**************************************************************************************/

    template <class I> auto const & operator[](I i) const {
        static_assert(i >= 0_c && i < size, "variant out of range");
        if (i != status) throw variant_error<I>(status);
        return static_cast<decltype(*types[i]) const *>(ptr())->value();
    }

    template <class I> auto & operator[](I i) {
        static_assert(i >= 0_c && i < size, "variant out of range");
        if (i != status) throw variant_error<I>(status);
        return static_cast<decltype(*types[i]) *>(ptr())->value();
    }

    /**************************************************************************************/

    template <class I, class ...Ts>
    void emplace(I i, Ts &&...ts) {
        fold(0_c, status, destroy(), *this);
        status = i;
        new(ptr()) decltype(*types[i]){std::forward<Ts>(ts)...};
    }

    /**************************************************************************************/

    template <class ...Ts>
    decltype(auto) visit(Ts &&...ts) {return fold(0_c, status, at(), *this, std::forward<Ts>(ts)...);}

    template <class ...Ts>
    decltype(auto) visit(Ts &&...ts) const {return fold(0_c, status, at(), *this, std::forward<Ts>(ts)...);}

    /**************************************************************************************/

    ~variant() {fold(0_c, status, destroy(), *this);}
};

template <class ...Types> using recursive_variant = variant<true, Types...>;
template <class ...Types> using nonrecursive_variant = variant<false, Types...>;

template <bool B=false> static constexpr auto variant_c = hana::fuse(hana::template_<recursive_variant>);
template <> static constexpr auto variant_c<> = hana::fuse(hana::template_<nonrecursive_variant>);


}
