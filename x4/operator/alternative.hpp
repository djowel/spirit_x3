#pragma once

namespace x4 {

template <class Left, class Right>
struct alternative_op {
    template <class Range>
    auto check(Range &r) {
        optional<data_type> ret;
        if (!check_set(ret, this->left)) check_set(ret, this->right);
        return ret;
    }

    value_type parse(data_type data) {
        if (data[0_c]) return this->left.parse(std::move(data[0_c]));
        else return this->left.parse(std::move(data[1_c]));
    }
};

}
