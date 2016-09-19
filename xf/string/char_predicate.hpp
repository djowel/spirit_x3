#pragma once
#include "../support/common.hpp"

namespace xf {

/******************************************************************************************/

template <class Predicate>
class char_predicate : parser_base {
    Predicate predicate;

public:

    template <bool B=true, int_if<std::is_default_constructible<Predicate>::value> = 0>
    constexpr char_predicate() {}

    explicit constexpr char_predicate(Predicate p) : predicate(std::move(p)) {}

    template <class Window>
    auto operator()(check_base, Window &w) const {
        if (bool(w) && predicate(*w)) return *(w++);
        else return static_cast<std::decay_t<decltype(*w)>>(0);
    }

    template <class T>
    constexpr auto operator()(parse_base, T t) const {return t;}
};

static constexpr auto char_predicate_c = hana::template_<char_predicate>;

/******************************************************************************************/

#define xf_CHAR_FUNCTOR(name, chop, wop, op) \
    struct name##_t { \
        auto operator()(char const t) const {return chop(t);} \
        auto operator()(wchar_t const t) const {return wop(t);} \
        template <class T> constexpr auto operator()(T &&t) const -> decltype(op(std::forward<T>(t))) {return op(std::forward<T>(t));} \
    }; \
    static constexpr auto name = name##_t(); \

#define xf_CHAR_PREDICATE(name, chop, wop, op) xf_CHAR_FUNCTOR(name, chop, wop, op) \
    static constexpr auto name##_x = char_predicate<name##_t>(name);

    xf_CHAR_PREDICATE(alpha,  std::isalpha,  std::iswalpha,  isalpha);
    xf_CHAR_PREDICATE(digit,  std::isdigit,  std::iswdigit,  isdigit);
    xf_CHAR_PREDICATE(xdigit, std::isxdigit, std::iswxdigit, isxdigit);
    xf_CHAR_PREDICATE(cntrl,  std::iscntrl,  std::iswcntrl,  iscntrl);
    xf_CHAR_PREDICATE(graph,  std::isgraph,  std::iswgraph,  isgraph);
    xf_CHAR_PREDICATE(space,  std::isspace,  std::iswspace,  isspace);
    xf_CHAR_PREDICATE(blank,  std::isblank,  std::iswblank,  isblank);
    xf_CHAR_PREDICATE(print,  std::isprint,  std::iswprint,  isprint);
    xf_CHAR_PREDICATE(punct,  std::ispunct,  std::iswpunct,  ispunct);
    xf_CHAR_PREDICATE(lower,  std::islower,  std::iswlower,  islower);
    xf_CHAR_PREDICATE(upper,  std::isupper,  std::iswupper,  isupper);

    xf_CHAR_FUNCTOR(to_lower,  std::tolower,  std::towlower, tolower);
    xf_CHAR_FUNCTOR(to_upper,  std::toupper,  std::towupper, toupper);

#undef xf_CHAR_PREDICATE
#undef xf_CHAR_FUNCTOR

static constexpr auto char_x = decltype(*char_predicate_c(type_of(always_true)))(always_true);

/******************************************************************************************/

struct caseless_compare {
    template <class T, class U>
    constexpr auto operator()(T t, U u) const {return to_lower(t) == to_lower(u);}
};

/******************************************************************************************/

}
