/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <x3/core/parser.hpp>
#include <x3/core/skip_over.hpp>
#include <x3/support/traits/move_to.hpp>
#include <x3/support/no_case.hpp>

namespace x3 {
    ///////////////////////////////////////////////////////////////////////////
    // The base char_parser
    ///////////////////////////////////////////////////////////////////////////
    template <typename Derived>
    struct char_parser : parser<Derived> {
        template <typename Iterator, typename Context, typename Attribute>
        bool parse(Iterator& first, Iterator const& last , Context const& context, unused_type, Attribute& attr) const {
            x3::skip_over(first, last, context);
            if (first != last && this->derived().test(*first, context)) {
                x3::traits::move_to(*first, attr);
                ++first;
                return true;
            }
            return false;
        }

        // Range contains bool() saying if it is empty, and does skippy stuff inside.
        //maybe<T> check(Range &r) const {return r ? *(r++) : {};}
        //auto parse(T &&c) const {return std::move(c);}
    };
}
