/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <x3/support/traits/attribute_of.hpp>
#include <x3/support/traits/has_attribute.hpp>
#include <x3/char/char_parser.hpp>

namespace x3 {
    ///////////////////////////////////////////////////////////////////////////
    // negated_char_parser handles ~cp expressions (cp is a char_parser)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Positive>
    struct negated_char_parser : char_parser<negated_char_parser<Positive>> {
        negated_char_parser(Positive const& positive) : positive(positive) {}

        template <typename CharParam, typename Context>
        bool test(CharParam ch, Context const& context) const {return !positive.test(ch, context); }

        Positive positive;
    };

    template <typename Positive>
    inline negated_char_parser<Positive>
    operator~(char_parser<Positive> const& cp) {return { cp.derived() }; }

    template <typename Positive>
    inline Positive const& operator~(negated_char_parser<Positive> const& cp) {return cp.positive; }
}

namespace x3 { namespace traits
{
    template <typename Positive, typename Context>
    struct attribute_of<x3::negated_char_parser<Positive>, Context> : attribute_of<Positive, Context> {};

    template <typename Positive, typename Context>
    struct has_attribute<x3::negated_char_parser<Positive>, Context> : has_attribute<Positive, Context> {};
}}
