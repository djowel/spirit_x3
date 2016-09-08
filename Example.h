template <class T> struct define_t;

class Integer;
template <> struct define_t<typename std::decay_t<decltype(Integer)>::id> {template <class=void> static constexpr auto definition = false;};
define_t<typename std::decay_t<decltype(Integer)>::id> get_define_t(typename std::decay_t<decltype(Integer)>::id *);
template <> auto const define_t<typename std::decay_t<decltype(Integer)>::id>::definition<> = x3::int_ && [](int i) {return i;};


class Addition;
template <> struct define_t<typename std::decay_t<decltype(Addition)>::id> {template <class=void> static constexpr auto definition = false;};
define_t<typename std::decay_t<decltype(Addition)>::id> get_define_t(typename std::decay_t<decltype(Addition)>::id *);
template <> auto const define_t<typename std::decay_t<decltype(Addition)>::id>::definition<> = rule(integer) >> rule(Integer) && [](int i, int j) {return i + j;};


DEF(addition) = (integer >> integer) & [](int i, int j, std::string aux) {print(aux); return i + j;}
              | (double_ >> double_) & [](double i, double j) {return i + j;};

DEF(addition) = (integer >> integer | double_ >> double_) & [](auto i, auto j) {return i + j;};

PREVIOUS
bool parse(Iterator& first, Iterator const& last, Context const& context, unused_type, Attribute& attr)

 For digit parser e.g.
auto parse(Character_Range &&range)
 For digit parser



parse(std::string, rule)
