/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman
    Copyright (c) 2001-2011 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#pragma once

#include <x3/core/proxy.hpp>
#include <x3/core/detail/parse_into_container.hpp>
#include <x3/support/traits/attribute_of.hpp>
#include <x3/support/traits/move_to.hpp>
#include <x3/support/traits/optional_traits.hpp>
#include <x3/support/traits/attribute_category.hpp>

namespace xf {

template <class Subject>
struct optional_op {
    Subject subject;

    template <class Range>
    auto check() const {return always(subject.check());}
};

}



namespace x3
{
    template <typename Subject>
    struct optional : proxy<Subject, optional<Subject>>
    {
        typedef proxy<Subject, optional<Subject>> base_type;
        static bool const handles_container = true;

        optional(Subject const& subject)
          : base_type(subject) {}

        using base_type::parse_subject;

        // Attribute is a container
        template <typename Iterator, typename Context
          , typename RContext, typename Attribute>
        bool parse_subject(Iterator& first, Iterator const& last
          , Context const& context, RContext& rcontext, Attribute& attr
          , traits::container_attribute) const
        {
            detail::parse_into_container(
                this->subject, first, last, context, rcontext, attr);
            return true;
        }

        // Attribute is an optional
        template <typename Iterator, typename Context
          , typename RContext, typename Attribute>
        bool parse_subject(Iterator& first, Iterator const& last
          , Context const& context, RContext& rcontext, Attribute& attr
          , traits::optional_attribute) const
        {
            typedef typename
                x3::traits::optional_value<Attribute>::type
            value_type;

            // create a local value
            value_type val = value_type();

            if (this->subject.parse(first, last, context, rcontext, val))
            {
                // assign the parsed value into our attribute
                x3::traits::move_to(val, attr);
            }
            return true;
        }
    };

    template <typename Subject>
    inline optional<typename extension::as_parser<Subject>::value_type>
    operator-(Subject const& subject)
    {
        return { as_parser(subject) };
    }
}

namespace x3 { namespace traits
{
    template <typename Subject, typename Context>
    struct attribute_of<x3::optional<Subject>, Context>
        : build_optional<
            typename attribute_of<Subject, Context>::type> {};
}}


