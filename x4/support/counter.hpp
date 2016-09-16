#pragma once
#include <boost/hana/map.hpp>

namespace x4 {
namespace hana = boost::hana;

/******************************************************************************************/

template <class Map>
class counter {
    Map map;

public:

    template <class T>
    auto count(T t) const {return hana::find(map, t).value_or(0_c);}

    template <class T>
    auto plus(T t) const {return map.erase(t).insert(t, count() + 1_c);}

    template <class T>
    auto zero(T t) const {return map.erase(t).insert(t, 0_c);}
};

/******************************************************************************************/

struct check_base {};

template <class T>
using map_type = std::conditional_t<std::is_same<void, T>::value, decltype(hana::make_map()), T>;

template <class Map=void>
class check_map : counter<map_type<Map>>, public check_base {
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
class parse_map : counter<map_type<Map>>, public parse_base {
    using base = counter<map_type<Map>>;
public:

    template <class T>
    auto plus(T t) const {return parse_map<decltype(base::plus(t))>();}

    template <class T>
    auto zero(T t) const {return parse_map<decltype(base::zero(t))>();}
};

template <class T> static constexpr auto is_parse = hana::bool_c<std::is_base_of<parse_base, T>::value>;

/******************************************************************************************/

}
