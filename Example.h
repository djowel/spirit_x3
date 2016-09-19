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

template <class Rule>
 auto parse(std::string s, Rule r) {
    auto args = r.pattern(s);
    if (args.pass) return r.transform(std::move(args));
 }

struct Rule {
    pattern_t pattern;
    transform_t transform;

    bool check() {
        auto args = r.pattern(s);
    }

    auto parse(std::string) {
        auto args = r.pattern(s);
        if (args) return r.transform(std::move(*args));
    }

};





parse(std::string, rule)

[](auto x) {if (x < 2) throw unmatched; return maybe(x);}

OPTION 2***
// check to see if valid, return all data necessary to reconstruct
auto check() {
    optional<variant<check1, check2>> data;
    data = parse1.check();
    if (!data) data = parse2.check();
    return data;
}

// using the previous data construct the return value
auto take(variant<check1, check2> &&data, auto &&args) {
    return parse1.take(std::move(data));
    return parse2.take(std::move(data));
}

// in the opaque lambda case, check returns the value and take just returns std::move() of it

// OPTION 2 [not sure about RVO, also exception overhead]
try {return parse1();}
catch (unmatched) {return parse2();} // throws if neither match

// OPTION 3 [possibly not as efficient if object construction is expensive]
bool parse(int &value) {
    if (whatever) return false;
    value = 3; return true;
}

// OPTION 4 [same as 3 but may have some issue with return value optimization]
optional<int> parse() {
    if (whatever) return {};
    else return 3;
}

attribute...
parse(Iterator& first , Iterator last , Parser const& p , Attribute& attr)

parse_main() {

}

template <class T>
struct char_parser {
    maybe<T> check(Iterator &r) const {return r ? *(r++) : {};}
    auto parse(T &&c) const {return std::move(c);}
};

template <class
