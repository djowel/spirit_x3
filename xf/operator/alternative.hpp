#pragma once
#include "../support/optional_variant.hpp"
#include "../parse/parse.hpp"
#include <boost/hana/for_each.hpp>

namespace xf {

static constexpr auto recurse_limit = 0_c;

/******************************************************************************************/

template <class Parser, class Window>
class alternative_check_t {
    using value_type = decltype(check(check_c, std::declval<Parser const &>(), std::declval<Window &>()));
    value_type m_value;
public:
    value_type & value() {return m_value;}
    value_type const & value() const {return m_value;}

    alternative_check_t(value_type v) : m_value(std::move(v)) {}
    alternative_check_t(Parser const &p, Window &w) : m_value(check(check_c, p, w)) {}
};

template <class Parser, class Data>
struct alternative_parse_t {
    using value_type = decltype(parse(parse_c, std::declval<Parser const &>(), std::declval<Data &&>()));
    value_type m_value;
public:
    value_type & value() {return m_value;}
    value_type const & value() const {return m_value;}

    alternative_parse_t(value_type v) : m_value(std::move(v)) {}
    alternative_parse_t(Parser const &p, Data &&d) : m_value(parse(parse_c, p, std::move(d))) {}
};

static auto constexpr alternative_parse_c = hana::template_<alternative_parse_t>;

/******************************************************************************************/

template <class ...Parsers>
class alternative : public parser_base {
    hana::tuple<Parsers...> m_parsers;

    template <class ...Ts> struct ID;

    template <bool B, class Tag, class Window, int_if<!B> = 0>
    auto check_type(Tag tag, Window &w) const {
        return optional_variant_c<false>(hana::transform(m_parsers, [&](auto const &p) {return type_of(check(tag, p, w));}));
    }
    template <bool B, class Tag, class Window, int_if<B> = 0>
    auto check_type(Tag, Window &w) const {
        return hana::type_c<optional_variant<true, alternative_check_t<Parsers, Window>...>>;
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

    template <bool B, class Tag, class Data, int_if<!B> = 0>
    auto parse_type(Tag tag, Data data) const {
        return variant_c<false>(hana::transform(indices_c<sizeof...(Parsers)>,
            [&](auto i) {return type_of(parse(tag, parsers()[i], std::move((*data)[i])));}));
    }
    template <bool B, class Tag, class Data, int_if<B> = 0>
    auto parse_type(Tag, Data data) const {
        return variant_c<true>(hana::transform(indices_c<sizeof...(Parsers)>,
            [&](auto i) {return alternative_parse_c(type_of(parsers()[i]), type_of((*data)[i]));}));
    }
    template <class R, class Tag, class Data>
    R do_parse(Tag tag, Data data) const {
        return data->visit([&](auto i, auto &d) {return R(i, parse(tag, m_parsers[i], std::move(d)));});
    }

public:

    auto const & parsers() const {return m_parsers;}

    template <class ...Ts>
    constexpr alternative(Ts &&...ts) : m_parsers(std::forward<Ts>(ts)...) {}

    template <class Tag, class Window, int_if<is_check<Tag>> = 0>
    auto operator()(Tag const tag, Window &w) const {
        constexpr auto id = hana::type_c<ID<Window>>;
        auto cycle = (tag.count(id) >= recurse_limit);
        auto tag2 = hana::if_(cycle, check_c, tag.plus(id));
        using R = decltype(*check_type<cycle>(tag2, w));
        return do_check<R>(tag2, w);
    }

    template <class Tag, class Data, int_if<is_parse<Tag>> = 0>
    auto operator()(Tag const tag, Data &&data) const {
        constexpr auto id = hana::type_c<ID<std::decay_t<Data>>>;
        auto cycle = (tag.count(id) >= recurse_limit);
        auto tag2 = hana::if_(cycle, parse_c, tag.plus(id));
        return do_parse<decltype(*parse_type<cycle>(tag2, std::forward<Data>(data)))>(tag2, std::forward<Data>(data));
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
