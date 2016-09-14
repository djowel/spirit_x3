#pragma once
#include "../parse/common.hpp"
#include <boost/hana/for_each.hpp>
#include <boost/hana/prepend.hpp>
#include <boost/hana/insert.hpp>
#include <boost/hana/insert_range.hpp>
#include <boost/hana/remove_if.hpp>
#include <utility>

namespace x4 {

/******************************************************************************************/

template <class Iterator, class Compare, class ...Masks>
class window;

/******************************************************************************************/

template <class Iterator, class Compare>
class window<Iterator, Compare> {
    Iterator m_begin, m_end;
    Compare m_compare;
protected:
    auto & base() {return *this;}
public:

    constexpr auto types() const {return hana::tuple_t<Iterator, Compare>;}

    template <class F> auto no_skip(F &&f) {return f(*this);}

    constexpr window(Compare c, Iterator b, Iterator e) : m_begin(b), m_end(e), m_compare(std::move(c)) {}

    template <bool B=true, int_if<std::is_default_constructible<Compare>::value> = 0>
    constexpr window(Iterator b, Iterator e) : m_begin(b), m_end(e) {}

    constexpr window(window w, hana::tuple<>) : window(std::move(w)) {}

    constexpr auto const & compare() const {return m_compare;}
    constexpr auto begin() const {return m_begin;}
    constexpr auto end() const {return m_end;}
    constexpr auto masks() const {return hana::tuple<>();}

    void set_begin(Iterator b) {m_begin = std::move(b);}
    void set_end(Iterator e) {m_end = std::move(e);}

    decltype(auto) operator++() {++m_begin; return *this;}

    auto operator++(int) {return m_begin++;}
    decltype(auto) operator*() const {return *m_begin;}

    constexpr operator bool() const {return m_begin != m_end;}
};

/******************************************************************************************/

template <class Iterator, class Compare, class ...Masks>
class window : public window<Iterator, Compare> {
    hana::tuple<Masks...> m_masks;

    void skip() {
        while (true) {
            bool changed = false;
            hana::for_each(m_masks, [&](auto const &m) {
                while (success_of(m, check_of(m, this->base()))) changed = true;
            });
            if (sizeof...(Masks) < 2 || !changed) break;
        }
    }

public:

    constexpr auto types() const {return hana::tuple_t<Iterator, Compare, Masks...>;}

    constexpr decltype(auto) masks() const {return masks;}

    template <class ...Ts>
    constexpr window(window<Iterator, Compare> w, Ts &&...ts) : window<Iterator, Compare>(std::move(w)), m_masks(std::forward<Ts>(ts)...) {skip();}

    template <class F> auto no_skip(F &&f) {
        auto ret = f(this->base());
        skip();
        return ret;
    }

    auto & operator++() {++this->base(); skip(); return *this;}
    auto operator++(int) {auto ret = this->begin(); ++(*this); return ret;}
};

static constexpr auto window_c = hana::fuse(hana::template_<window>);

/******************************************************************************************/

// Add a mask
template <class Iterator, class Compare, class Mask, class ...Masks>
auto with_mask(window<Iterator, Compare, Masks...> const &w, Mask &&m) {
    using R = decltype(*window_c(hana::insert(w.types(), 2_c, hana::decltype_(m))));
    return R(w, hana::prepend(w.masks(), std::forward<Mask>(m)));
}

template <class Iterator, class Compare, class Mask, class ...Masks>
auto with_masks(window<Iterator, Compare, Masks...> const &w, Mask &&m) {
    using R = decltype(*window_c(hana::insert_range(w.types(), 2_c, types_of(m))));
    return R(w, hana::concat(std::forward<Mask>(m), w.masks()));
}

// Remove masks satisfying a predicate
template <class Iterator, class Compare, class Predicate, class ...Masks>
auto without_mask(window<Iterator, Compare, Masks...> const &w, Predicate rm) {
    auto masks = hana::remove_if(w.masks(), std::forward<Predicate>(rm));
    using R = decltype(*window_c(hana::concat(hana::tuple_t<Iterator, Compare>, types_of(masks))));
    return R(w, std::move(masks));
}

/******************************************************************************************/

template <class V, class Compare=decltype(hana::equal) const &>
auto make_window(V const &v, Compare &&cmp=hana::equal) {
    return window<std::decay_t<decltype(v.begin())>, std::decay_t<Compare>>(std::forward<Compare>(cmp), v.begin(), v.end());
}


}
