#pragma once

#include <boost/hana/prepend.hpp>
#include <boost/hana/plus.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/minus.hpp>

#include "variant.hpp"
#include <utility>

namespace x4 {

/******************************************************************************************/

template <class ...Types>
class optional_variant : public variant<decltype(*nothing_c), Types...> {
    using base = variant<decltype(*nothing_c), Types...>;
public:
    optional_variant() : base(0_c) {}

    auto index() const {return base::index() - 1_c;}
    constexpr operator bool() const {return base::index();}

    template <class I> decltype(auto) operator[](I i) {return base::operator[](i + 1_c);}
    template <class I> decltype(auto) operator[](I i) const {return base::operator[](i + 1_c);}

    template <class I, class ...Ts>
    void emplace(I i, Ts &&...ts) {base::emplace(i + 1_c, std::forward<Ts>(ts)...);}

    template <class F> constexpr auto visit(F &&f) const {
        return base::visit([&](auto i, auto const &t) {return std::forward<F>(f)(i-1_c, t);}, 1_c);
    }

    template <class F> constexpr auto visit(F &&f) {
        return base::visit([&](auto i, auto &t) {return std::forward<F>(f)(i-1_c, t);}, 1_c);
    }
};

static constexpr auto optional_variant_c = hana::fuse(hana::template_<optional_variant>);

/******************************************************************************************/

}
