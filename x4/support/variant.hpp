#pragma once
#include <boost/hana/traits.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/not_equal.hpp>

namespace x4 {

template <class ...Types>
using aligned_union_t = typename std::aligned_union<1024, Types...>::type;

static constexpr auto aligned_union = hana::fuse(hana::template_<aligned_union_t>);

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

    template<bool B=true, int_if<B> = 0>
    recursive_wrap(recursive_wrap const &other) : ptr(new T(*other.ptr)) {}

    template<bool B=true, int_if<B> = 0>
    recursive_wrap &operator=(recursive_wrap const &other) {*ptr = *other.ptr; return *this;}

    void swap(recursive_wrap &other) {ptr.swap(other.ptr);}

    ~recursive_wrap() {std::cout << "hmm" << std::endl;}
};

template <class T>
void swap(recursive_wrap<T> &r1, recursive_wrap<T> &r2) {r1.swap(r2);}

/******************************************************************************************/

template <class Index>
class variant_error : std::exception {
    int n;
public:
    explicit variant_error(int n_) : n(n_) {}

    char const * what() const noexcept override {return "variant error";}
};

/******************************************************************************************/

struct placement_destroy {
    template <class I, class T, int_if<!std::is_class<T>::value> = 0>
    void operator()(I i, T const &) const {}

    template <class I, class T, int_if<std::is_class<T>::value> = 0>
    void operator()(I i, T &t) const {t.~T();}
};

struct placement_move_t {
    template <class I, class T>
    void operator()(void *data, I, T &&t) const {new(data) T(std::move(t));}
};

static constexpr auto placement_move = hana::curry<3>(placement_move_t());

struct placement_copy_t {
    template <class T, class I>
    void operator()(void *data, I, T const &t) const {
        std::cout << typeid(T).name() << std::endl;
        new(data) T(std::move(t));}
};

static constexpr auto placement_copy = hana::curry<3>(placement_copy_t());

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

    template <class I> using type_at = decltype(*types[std::declval<I>()]);

    decltype(*storage) data;
    std::conditional_t<sizeof...(Types) <= 1, decltype(size), int> n;

    template <class I>
    void check(I i) const {if (i != n) throw variant_error<I>(n);}

    /**************************************************************************************/

    auto ptr() const {return static_cast<void const *>(std::addressof(data));}
    template <class I> auto const & cast(I i) const {return static_cast<decltype(*types[i]) const *>(ptr())->value();}

    /**************************************************************************************/

    auto ptr() {return static_cast<void *>(std::addressof(data));}
    template <class I> auto & cast(I i) {return static_cast<decltype(*types[i]) *>(ptr())->value();}

    static auto can_construct(hana::type<variant>) {return hana::false_c;}

    template <class I, class ...Ts, int_if<I::value >= 0> = 0>
    static auto can_construct(I i, Ts ...ts) {return hana::traits::is_constructible(types[i], ts...);}

public:

    constexpr auto which() const {return n;}

    template <bool B=true, int_if<B && decltype(can_default)::value> = 0>
    constexpr variant() {
        constexpr auto N = index_if(types, hana::traits::is_default_constructible);
        new(ptr()) decltype(*types[N]){};
        n = N;
        std::cout << int(n) << int(N) << std::endl;
        auto v = cast(N);
        std::cout << int(n) << int(N) << std::endl;
    }

    template <class I, class ...Ts, int_if<decltype(can_construct(std::declval<I>(), hana::type_c<Ts &&>...))::value> = 0>
    explicit variant(I i, Ts &&...ts) : n(i) {new(ptr()) decltype(*types[i])(std::forward<Ts>(ts)...);}

    /**************************************************************************************/

    //template <bool B=true, int_if<B && decltype(can_move)::value> = 0>
    variant(variant &&v) : n(v.n) {v.visit(placement_move(ptr()));}

    //template <bool B=true, int_if<B && decltype(can_copy)::value> = 0>
    variant(variant const &v) : n(v.n) {
        std::cout << "copy" << std::endl;
        assert(n < 10);
        v.visit(placement_copy(ptr()));
        std::cout << "end copy" << std::endl;
    }

    //template <bool B=true, int_if<B && decltype(can_move || can_copy)::value> = 0>
    variant & operator=(variant other) {std::swap(data, other.data); return *this;}

    /**************************************************************************************/

    template <class I>
    auto const & operator[](I i) const {check(i); return cast(i);}

    template <class I>
    auto & operator[](I i) {check(i); return cast(i);}

    /**************************************************************************************/

    template <class F, class I=decltype(0_c), int_if<I::value + 1 >= sizeof...(Types)> = 0>
    constexpr decltype(auto) visit(F &&f, I i=0_c) {
    std::cout << "nconst1 " << int(i) << " " << int(n) << std::endl;

    assert(i == n); return f(i, cast(i)); }

    template <class F, class I=decltype(0_c), int_if<I::value + 2 <= sizeof...(Types)> = 0>
    constexpr decltype(auto) visit(F &&f, I i=0_c) {
        std::cout << "nconst " << int(i) << " " << int(n) << std::endl;

        return n == i ? f(i, cast(i)) : visit(std::forward<F>(f), i + 1_c);
    }

    template <class F, class I=decltype(0_c), int_if<I::value + 1 >= sizeof...(Types)> = 0>
    constexpr decltype(auto) visit(F &&f, I i=0_c) const {
        std::cout << "const1 " << int(i) << " " << int(n) << std::endl;
        assert(i == n);
        return f(i, cast(i));
    }

    template <class F, class I=decltype(0_c), int_if<I::value + 2 <= sizeof...(Types)> = 0>
    constexpr decltype(auto) visit(F &&f, I i=0_c) const {
        std::cout << "const " << int(i) << " " << int(n) << std::endl;
        std::cout << (n == i) << std::endl;
        auto v = cast(i);
        std::cout << "good" << std::endl;
        return n == i ? f(i, cast(i)) : visit(std::forward<F>(f), i + 1_c);
    }


    /**************************************************************************************/

    template <class I, class ...Ts>
    void emplace(I i, Ts &&...ts) {visit(placement_destroy()); n = i; new(ptr()) decltype(*types[i])(std::forward<Ts>(ts)...); }

    /**************************************************************************************/

    ~variant() {
        assert(n < 1000);
        visit(placement_destroy());
    }
};



}
