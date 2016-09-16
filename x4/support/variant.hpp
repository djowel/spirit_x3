#pragma once
#include <boost/hana/traits.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/greater_equal.hpp>

#include "recursive.hpp"

namespace x4 {

template <class ...Types>
using aligned_union_t = typename std::aligned_union<1024, Types...>::type;
static constexpr auto aligned_union = hana::fuse(hana::template_<aligned_union_t>);

/******************************************************************************************/

template <class Index>
class variant_error : std::exception {
    int n;
public:
    explicit variant_error(int n_) : n(n_) {}

    char const * what() const noexcept override {return "variant error";}
};

/******************************************************************************************/

template <class V, class P>
constexpr auto index_if(V const &v, P &&p) {
    constexpr auto first = hana::reverse_partial(hana::at, 0_c);
    constexpr auto second = hana::reverse_partial(hana::at, 1_c);
    return first(hana::find_if(enumerate(v), hana::compose(std::forward<P>(p), second)).value());
}

/******************************************************************************************/

template <class ...Types>
class variant {

    static constexpr auto size = hana::int_c<sizeof...(Types)>;
    static constexpr auto types = hana::tuple_t<recursive_wrap<Types>...>;
    static constexpr auto storage = aligned_union(types);

    static constexpr auto can_move = hana::all_of(types, hana::traits::is_move_constructible);
    static constexpr auto can_copy = hana::all_of(types, hana::traits::is_copy_constructible);
    static constexpr auto can_default = hana::any_of(types, hana::traits::is_default_constructible);
    static constexpr auto can_swap = hana::true_c;//hana::all_of(types, hana::traits::is_swappable);

    template <class I, class ...Ts, int_if<I::value >= 0> = 0>
    static constexpr auto can_construct(I i, Ts ...ts) {return hana::traits::is_constructible(types[i], ts...);}

    decltype(*storage) data;
    std::conditional_t<size <= 1, decltype(size), int> status;

    /**************************************************************************************/

    auto ptr() const {return static_cast<void const *>(std::addressof(data));}
    auto ptr() {return static_cast<void *>(std::addressof(data));}

    /**************************************************************************************/

    template <class F, class I, class N, class ...Ts, int_if<I::value + 1 >= size> = 0>
    static decltype(auto) visit(I i, N n, F &&f, Ts &&...ts) {assert(i == n); return f(i, std::forward<Ts>(ts)...);}

    template <class F, class I, class N, class ...Ts, int_if<I::value + 2 <= size> = 0>
    static decltype(auto) visit(I i, N n, F &&f, Ts &&...ts) {
        if (n == i) return f(i, std::forward<Ts>(ts)...);
        else visit(i + 1_c, n, std::forward<F>(f), std::forward<Ts>(ts)...);
    }

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
        template <class I, int_if<hana::traits::is_class(types[I()])> = 0>
        void operator()(I i, variant &self) const {
            using T = decltype(*types[i]);
            static_cast<T *>(self.ptr())->~T();
        }

        template <class I, int_if<!hana::traits::is_class(types[I()])> = 0>
        void operator()(I i, variant &self) const {}
    };

public:

    constexpr auto index() const {return status;}

    template <bool B=true, int_if<B && decltype(can_default)::value> = 0>
    constexpr variant() {
        constexpr auto N = index_if(types, hana::traits::is_default_constructible);
        new(ptr()) decltype(*types[N]){};
        status = N;
    }

    template <class I, class ...Ts, int_if<decltype(can_construct(std::declval<I>(), hana::type_c<Ts &&>...))::value> = 0>
    explicit variant(I i, Ts &&...ts) : status(i) {new(ptr()) decltype(*types[i])(std::forward<Ts>(ts)...);}

    /**************************************************************************************/

    template <bool B=true, int_if<B && can_move> = 0>
    variant(variant &&v) : status(v.status) {visit(0_c, status, move(), *this, v);}

    template <bool B=true, int_if<B && can_copy> = 0>
    variant(variant const &v) : status(v.status) {visit(0_c, status, copy(), *this, v);}

    friend void swap(variant &v1, variant &v2) {std::swap(v1.data, v2.data); std::swap(v1.status, v2.status);}

    template <bool B=true, int_if<B && (can_move || can_copy)> = 0>
    variant & operator=(variant other) {swap(*this, other); return *this;}

    /**************************************************************************************/

    template <class I> auto const & operator[](I i) const {
        if (i != status) throw variant_error<I>(status);
        return static_cast<decltype(*types[i]) const *>(ptr())->value();
    }

    template <class I> auto & operator[](I i) {
        if (i != status) throw variant_error<I>(status);
        return static_cast<decltype(*types[i]) *>(ptr())->value();
    }

    /**************************************************************************************/

    template <class I, class ...Ts>
    void emplace(I i, Ts &&...ts) {
        visit(0_c, status, destroy(), *this);
        status = i;
        new(ptr()) decltype(*types[i]){std::forward<Ts>(ts)...};
    }

    /**************************************************************************************/

    ~variant() {visit(0_c, status, destroy(), *this);}
};

static constexpr auto variant_c = hana::fuse(hana::template_<variant>);

}
