/*=============================================================================
    Copyright (c) 2015 Mario Lang
    Copyright (c) 2001-2011 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#pragma once

#include <x3/core/parser.hpp>
#include <x3/support/traits/move_to.hpp>
#include <x3/support/unused.hpp>

namespace x3 {
    template <typename Subject>
    struct matches_directive : unary_parser<Subject, matches_directive<Subject>> {
        using base_type = unary_parser<Subject, matches_directive<Subject>>;
        static bool const has_attribute = true;
        using attribute_type = bool;

        matches_directive(Subject const& subject) : base_type(subject) {}

        template <typename Iterator, typename Context , typename RContext, typename Attribute>
        bool parse(Iterator& first, Iterator const& last , Context const& context, RContext& rcontext, Attribute& attr) const {
            bool const result = this->subject.parse(
                    first, last, context, rcontext, unused);
            traits::move_to(result, attr);
            return true;
        }
    };

    struct matches_gen {
        template <typename Subject>
        matches_directive<typename extension::as_parser<Subject>::value_type>
        operator[](Subject const& subject) const {return { as_parser(subject) }; }
    };

    auto const matches = matches_gen{};
}
