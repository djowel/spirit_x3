#pragma once

namespace x4 {

template <class Subject>
struct optional_op {
    Subject subject;

    template <class Range>
    auto check() const {return always(subject.check());}
};

}
