#pragma once
#include <boost/hana/map.hpp>
#include <boost/hana/erase_key.hpp>

namespace xf {
namespace hana = boost::hana;

/******************************************************************************************/

template <class Map>
class counter {
    Map map;

public:

    template <class T>
    auto count(T t) const {return hana::find(map, t).value_or(0_c);}

    template <class T>
    auto plus(T t) const {return hana::insert(hana::erase_key(map, t), hana::make_pair(t, count(t) + 1_c));}

    template <class T>
    auto zero(T t) const {return hana::erase_key(map, t);}
};

/******************************************************************************************/

struct check_base {};

template <class T>
using map_type = std::conditional_t<std::is_same<void, T>::value, decltype(hana::make_map()), T>;

template <class Map=void>
class check_map : public counter<map_type<Map>>, public check_base {
    using base = counter<map_type<Map>>;
public:

    template <class T>
    auto plus(T t) const {return check_map<decltype(base::plus(t))>();}

    template <class T>
    auto zero(T t) const {return check_map<decltype(base::zero(t))>();}
};

template <class T> static constexpr auto is_check = hana::bool_c<std::is_base_of<check_base, T>::value>;

/******************************************************************************************/

struct parse_base {};

template <class Map=void>
class parse_map : public counter<map_type<Map>>, public parse_base {
    using base = counter<map_type<Map>>;
public:

    template <class T>
    auto plus(T t) const {return parse_map<decltype(base::plus(t))>();}

    template <class T>
    auto zero(T t) const {return parse_map<decltype(base::zero(t))>();}
};

template <class T> static constexpr auto is_parse = hana::bool_c<std::is_base_of<parse_base, T>::value>;

/******************************************************************************************/

static constexpr auto check_c = check_map<>();
static constexpr auto parse_c = parse_map<>();

}
