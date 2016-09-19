#pragma once

#include <boost/hana/prepend.hpp>
#include <boost/hana/plus.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/minus.hpp>

#include "variant.hpp"
#include <utility>

namespace xf {

template <bool B, class ...Types> using optional_variant = boost::optional<variant<B, Types...>>;
template <class ...Types> using recursive_optional_variant = optional_variant<true, Types...>;
template <class ...Types> using nonrecursive_optional_variant = optional_variant<false, Types...>;

template <bool B=false> static constexpr auto optional_variant_c = hana::fuse(hana::template_<recursive_optional_variant>);
template <> static constexpr auto optional_variant_c<> = hana::fuse(hana::template_<nonrecursive_optional_variant>);

/******************************************************************************************/

}
