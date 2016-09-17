#pragma once
#include "../support/optional_variant.hpp"
#include "../parse/parse.hpp"
#include <boost/hana/for_each.hpp>

namespace x4 {

static constexpr auto recurse_limit = 0_c;

/******************************************************************************************/

//template <class V, class=void>
//struct reduce_variant {
//    template <class I, class T>
//    constexpr auto operator()(I i, T &&t) const {return V(i, std::forward<T>(t));}
//};
//
//template <class T, class ...Ts>
//struct reduce_variant<variant<T, Ts...>, std::enable_if_t<false && decltype(hana::tuple_t<T, Ts...> == hana::tuple_t<Ts..., T>)::value>> {
//    template <class I, class T_>
//    auto operator()(I, T_ &&t) const {return std::forward<T_>(t);}
//};

/******************************************************************************************/

struct alternative_base {
    template <class Parser, class Window>
    class check_t {
        using value_type = decltype(check(check_c, std::declval<Parser const &>(), std::declval<Window &>()));
        value_type m_value;

    public:

        value_type & value() {return m_value;}
        value_type const & value() const {return m_value;}

        template <class Tag>
        check_t(Tag tag, Parser const &p, Window &w) : m_value(check(tag, p, w)) {}

        check_t(value_type v) : m_value(std::move(v)) {}
    };

    template <class Parser, class Data, class ...Args>
    struct parse_t {
        using value_type = decltype(parse(parse_c, std::declval<Parser const &>(), std::declval<Data &&>(), std::declval<Args &&>()...));
        value_type m_value;

    public:

        value_type & value() {return m_value;}
        value_type const & value() const {return m_value;}

        template <class Tag>
        parse_t(Tag tag, Parser const &p, Data &&d, Args &&...args)
            : m_value(parse(tag, p, std::move(d), std::forward<Args>(args)...)) {}

        parse_t(value_type v) : m_value(std::move(v)) {}
    };

    static auto constexpr parse_ = hana::template_<parse_t>;
};

/// Everything is fine for [0..N) actually. No extra struct needed. Only needed every N values.
// or is it.
// I think so. we let normal recursion go for [0..N) then hijack into incomplete struct for the N
// Probably don't need tag then, since it is in general 0_c -- unless something weird? I think OK.

template <class ...Parsers>
class alternative : public parser_base, public alternative_base {
    hana::tuple<Parsers...> m_parsers;

    template <class ...Ts> struct ID;

    template <bool B, class Tag, class Window, int_if<!B> = 0>
    auto check_type(Tag tag, Window &w) const {
        return optional_variant_c<false>(hana::transform(m_parsers, [&](auto const &p) {return type_of(check(tag, p, w));}));
    }
    template <bool B, class Tag, class Window, int_if<B> = 0>
    auto check_type(Tag tag, Window &w) const {
        return hana::type_c<optional_variant<true, alternative_base::check_t<Parsers, Window>...>>;
    }

    template <class R, class Tag, class Window>
    R do_check(Tag tag, Window &w) const {
        R ret;
        hana::for_each(indices_c<sizeof...(Parsers)>, [&](auto i) {
            if (ret) return;
            auto t = check(tag, m_parsers[i], w);
            if (valid(m_parsers[i], t)) ret.emplace(i, std::move(t));
        });
        return ret;
    }

    template <bool B, class Tag, class Data, class ...Args, int_if<!B> = 0>
    auto parse_type(Tag tag, Data data, Args &&...args) const {
        return variant_c<false>(hana::transform(indices_c<sizeof...(Parsers)>,
            [&](auto i) {return type_of(parse(tag, parsers()[i], std::move(data[i]), std::forward<Args>(args)...));}));
    }
    template <bool B, class Tag, class Data, class ...Args, int_if<B> = 0>
    auto parse_type(Tag tag, Data data, Args &&...) const {
        return variant_c<true>(hana::transform(indices_c<sizeof...(Parsers)>,
            [&](auto i) {return alternative_base::parse_(type_of(parsers()[i]), type_of(data[i]), hana::type_c<Args>...);}));
    }
    template <class R, class Tag, class Data, class ...Args>
    R do_parse(Tag tag, Data data, Args &&...args) const {
        return data.visit([&](auto i, auto &d) {return R(i, parse(tag, m_parsers[i], std::move(d), std::forward<Args>(args)...));});
    }

public:

    auto const & parsers() const {return m_parsers;}

    template <class ...Ts>
    constexpr alternative(Ts &&...ts) : m_parsers(std::forward<Ts>(ts)...) {}

    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    auto operator()(Tag const tag, Window &w) const {
        constexpr auto id = hana::type_c<ID<Window>>;
        auto cycle = (tag.count(id) >= recurse_limit);
        auto tag2 = hana::if_(cycle, tag.zero(id), tag.plus(id));
        using R = decltype(*check_type<cycle>(tag2, w));
        return do_check<R>(tag2, w);
    }

    template <class Tag, class ...Args, int_if<is_parse<Tag>> = 0>
    auto operator()(Tag const tag, Args &&...args) const {
        constexpr auto id = hana::type_c<ID<Args...>>;
        auto cycle = (tag.count(id) >= recurse_limit);
        auto tag2 = hana::if_(cycle, tag.zero(id), tag.plus(id));
        using R = decltype(*parse_type<cycle>(tag2, std::forward<Args>(args)...));
        return do_parse<R>(tag2, std::forward<Args>(args)...);
    }
};

/******************************************************************************************/

static constexpr auto alternative_c = hana::fuse(hana::template_<alternative>);

template <class T> struct is_alternative_t : std::false_type {};
template <class ...Parsers> struct is_alternative_t<alternative<Parsers...>> : std::true_type {};
template <class T> static constexpr auto is_alternative = hana::bool_c<is_alternative_t<T>::value>;

template <class T> constexpr auto make_alternative(T &&t) {
    return decltype(*alternative_c(types_of(t)))(std::forward<T>(t));
}

/******************************************************************************************/

template <class L, class R, int_if<(is_expression<L> || is_expression<R>) && !(is_alternative<L> || is_alternative<R>)> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::make_tuple(expr(l), expr(r)));}

template <class L, class R, int_if<is_alternative<L> && !is_alternative<R>> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::append(l.parsers(), expr(r)));}

template <class L, class R, int_if<!is_alternative<L> && is_alternative<R>> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::prepend(r.parsers(), expr(l)));}

template <class L, class R, int_if<is_alternative<L> && is_alternative<R>> = 0>
constexpr auto operator|(L const &l, R const &r) {return make_alternative(hana::concat(l.parsers(), r.parsers()));}

/******************************************************************************************/

template <class ...Ts>
constexpr auto any(Ts &&...ts) {return make_alternative(hana::make_tuple(expr(std::forward<Ts>(ts))...));}

/******************************************************************************************/

}
