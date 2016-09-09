namespace x4 {

template <class Left, class Right>
struct sequence {
    Left left, Right right;

    template <class Range>
    auto check(Range r) const {
        variant_data<decltype(left.check()), decltype(right.check())> ret;
        if (!(ret = check_of(left))) ret = check_of(left);
        return ret;
    }
};

}
