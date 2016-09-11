#pragma once
#include <boost/hana/for_each.hpp>
#include <boost/hana/remove_if.hpp>
#include <utility>

namespace x4 {

/******************************************************************************************/

template <class Iterator, class ...Masks>
class window;

/******************************************************************************************/

template <class Iterator>
class window<Iterator> {
    Iterator m_begin, m_end;

public:
    constexpr window(Iterator b, Iterator e) : m_begin(b), m_end(e) {}

    auto begin() const {return m_begin;}
    auto end() const {return m_end;}
    constexpr auto masks() const {return hana::tuple<>();}

    decltype(auto) operator++() {++m_begin; return *this;}
    decltype(auto) operator*() const {return *m_begin;}

    constexpr operator bool() const {return m_begin != m_end;}
};

/******************************************************************************************/

template <class Iterator, class ...Masks>
class window : window<Iterator> {
    hana::tuple<Masks...> m_masks;

public:
    constexpr decltype(auto) masks() const {return masks;}

    template <class ...Ts>
    constexpr window(window<Iterator> w, Ts &&...ts) : window<Iterator>(std::move(w)), m_masks(std::forward<Ts>(ts)...) {}

    decltype(auto) operator++() {
        for (bool changed = true; ; (sizeof...(Masks) > 1) && std::exchange(changed, false)) {
            hana::for_each(m_masks, [&](auto const &m) {
                while (m.success_of(m.check(static_cast<window<Iterator> &>(*this)))) changed = true;
            });
        }
        return *this;
    }
};

static constexpr auto window_c = hana::fuse(hana::template_<window>);

/******************************************************************************************/

// Add a mask
template <class Iterator, class Mask, class ...Masks>
auto with_mask(window<Iterator, Masks...> const &w, Mask &&m) {
    using R = window<Iterator, std::decay_t<Mask>, Masks...>;
    return R(w, hana::prepend(w.masks(), std::forward<Mask>(m)));
}

// Remove masks satisfying a predicate
template <class Iterator, class Predicate, class ...Masks>
auto without_mask(window<Iterator, Masks...> const &w, Predicate rm) {
    auto masks = hana::remove_if(w.masks(), std::forward<Predicate>(rm));
    using R = decltype(*window_c(hana::prepend(hana::type_c<Iterator>, hana::transform(masks, hana::decltype_))));
    return R(w, std::move(masks));
}

/******************************************************************************************/

template <class V>
auto make_window(V const &v) {return window<std::decay_t<decltype(v.begin())>>(v.begin(), v.end());}


}
