/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <x3/auxiliary/guard.hpp>
#include <x3/core/parser.hpp>
#include <x3/core/skip_over.hpp>
#include <x3/directive/expect.hpp>
#include <x3/support/traits/make_attribute.hpp>
#include <x3/support/utility/sfinae.hpp>
#include <x3/nonterminal/detail/transform_attribute.hpp>
#include <boost/utility/addressof.hpp>

// Macro explanation:
// this can be declared repeatedly wherever since it's just forward declare
// specialize the struct to contain a definition member which is templated on bool (never encounter with bool=false)
// let X3 find the struct via ADL
// specialize the struct's member definition for case(true): I thought I needed template<> template<> but apparently not.

#define X3_DEFINE(rule) \
    template <class T> struct define_t; \
    template <> struct define_t<typename std::decay_t<decltype(rule)>::id> {template <class=void> static constexpr auto definition = false;}; \
    define_t<typename std::decay_t<decltype(rule)>::id> get_define_t(typename std::decay_t<decltype(rule)>::id *); \
    template <> auto const define_t<typename std::decay_t<decltype(rule)>::id>::definition<>


namespace x3 {
    template <typename ID>
    struct identity;

    template <typename ID, typename Attribute = unused_type, bool force_attribute = false>
    struct rule;

    struct parse_pass_context_tag;




    // Default case which should disable custom use
    int get_define_t(...);

    // Detect the custom definition: default false
    template <class ID, class=void> struct has_custom_definition : std::false_type {};
    // Detect the custom definition: look at declared return type of get_define_t(ID *). Resort to pointer, not declval<>() because incomplete type error I got
    template <class ID> struct has_custom_definition<ID, std::enable_if_t<(!std::is_same<int, decltype(get_define_t((ID *) nullptr))>::value)>> : std::true_type {};

    // This function will take over from old one for cases where define_t is specialized
    template <typename ID, typename Attribute, class I, class C, class A, std::enable_if_t<has_custom_definition<ID>::value, int> = 0>
    bool parse_rule(rule<ID, Attribute> rule, I& first, I const &last , C const &context, A &attr) {
        static auto const def = (rule = decltype(get_define_t((ID *) nullptr))::template definition<>);
        static_assert(std::pair<Attribute, A>::aaa, "");
        return def.parse(first, last, context, unused, attr);
    }




    namespace detail {
        // we use this so we can detect if the default parse_rule
        // is the being called.
        struct default_parse_rule_result {
            default_parse_rule_result(bool r) : r(r) {}
            operator bool() const { return r; }
            bool r;
        };
    }

    // default parse_rule implementation
    template <typename ID, typename Attribute, typename Iterator , typename Context, typename ActualAttribute, std::enable_if_t<!has_custom_definition<ID>::value, int> = 0>
    inline detail::default_parse_rule_result parse_rule(rule<ID, Attribute> rule_ , Iterator& first, Iterator const& last , Context const& context, ActualAttribute& attr); }

namespace x3 { namespace detail {
    template <typename ID, typename Iterator, typename Context, typename Enable = void>
    struct has_on_error : mpl::false_ {};

    template <typename ID, typename Iterator, typename Context>
    struct has_on_error<ID, Iterator, Context, typename disable_if_substitution_failure< decltype(std::declval<ID>().on_error(std::declval<Iterator&>() , std::declval<Iterator>() , std::declval<expectation_failure<Iterator>>() , std::declval<Context>() ) )>::type > : mpl::true_ {};

    template <typename ID, typename Iterator, typename Attribute, typename Context, typename Enable = void>
    struct has_on_success : mpl::false_ {};

    template <typename ID, typename Iterator, typename Attribute, typename Context>
    struct has_on_success<ID, Iterator, Context, Attribute, typename disable_if_substitution_failure< decltype(std::declval<ID>().on_success(std::declval<Iterator&>() , std::declval<Iterator>() , std::declval<Attribute&>() , std::declval<Context>() ) )>::type > : mpl::true_ {};

    template <typename ID>
    struct make_id {typedef identity<ID> type; };

    template <typename ID>
    struct make_id<identity<ID>> {typedef identity<ID> type; };

    template <typename ID, typename RHS, typename Context>
    Context const& make_rule_context(RHS const& rhs, Context const& context , mpl::false_ /* is_default_parse_rule */) {return context; }

    template <typename ID, typename RHS, typename Context>
    auto make_rule_context(RHS const& rhs, Context const& context , mpl::true_ /* is_default_parse_rule */ ) {return make_unique_context<ID>(rhs, context); }

    template <typename Attribute, typename ID>
    struct rule_parser {
        template <typename Iterator, typename Context, typename ActualAttribute>
        static bool call_on_success(Iterator& first, Iterator const& last , Context const& context, ActualAttribute& attr , mpl::false_ /* No on_success handler */ ) {return true; }

        template <typename Iterator, typename Context, typename ActualAttribute>
        static bool call_on_success(Iterator& first, Iterator const& last , Context const& context, ActualAttribute& attr , mpl::true_ /* Has on_success handler */) {
            bool pass = true;
            ID().on_success(first , last , attr , make_context<parse_pass_context_tag>(pass, context) );
            return pass;
        }

        template <typename RHS, typename Iterator, typename Context , typename RContext, typename ActualAttribute>
        static bool parse_rhs_main(RHS const& rhs , Iterator& first, Iterator const& last , Context const& context, RContext& rcontext, ActualAttribute& attr , mpl::false_) {
            // see if the user has a BOOST_SPIRIT_DEFINE for this rule
            typedef decltype(parse_rule(rule<ID, Attribute>(), first, last , make_unique_context<ID>(rhs, context), attr)) parse_rule_result;

            // If there is no BOOST_SPIRIT_DEFINE for this rule,
            // we'll make a context for this rule tagged by its ID
            // so we can extract the rule later on in the default
            // (generic) parse_rule function.
            typedef is_same<parse_rule_result, default_parse_rule_result> is_default_parse_rule;

            Iterator i = first;
            bool r = rhs.parse(i , last , make_rule_context<ID>(rhs, context, is_default_parse_rule()) , rcontext , attr );

            if (r) {
                auto first_ = first;
                x3::skip_over(first_, last, context);
                r = call_on_success(first_, i, context, attr , has_on_success<ID, Iterator, Context, ActualAttribute>());
            }

            if (r) first = i;
            return r;
        }

        template <typename RHS, typename Iterator, typename Context
          , typename RContext, typename ActualAttribute>
        static bool parse_rhs_main(RHS const& rhs , Iterator& first, Iterator const& last , Context const& context, RContext& rcontext, ActualAttribute& attr , mpl::true_ /* on_error is found */) {
            for (;;) {
                try {
                    return parse_rhs_main(rhs, first, last, context, rcontext, attr, mpl::false_());
                } catch (expectation_failure<Iterator> const& x) {
                    switch (ID().on_error(first, last, x, context)) {
                        case error_handler_result::fail: return false;
                        case error_handler_result::retry: continue;
                        case error_handler_result::accept: return true;
                        case error_handler_result::rethrow: throw;
                    }
                }
            }
        }

        template <typename RHS, typename Iterator , typename Context, typename RContext, typename ActualAttribute>
        static bool parse_rhs_main(RHS const& rhs , Iterator& first, Iterator const& last , Context const& context, RContext& rcontext, ActualAttribute& attr) {
            return parse_rhs_main(rhs, first, last, context, rcontext, attr , has_on_error<ID, Iterator, Context>() );
        }

        template <typename RHS, typename Iterator , typename Context, typename RContext, typename ActualAttribute>
        static bool parse_rhs(RHS const& rhs , Iterator& first, Iterator const& last , Context const& context, RContext& rcontext, ActualAttribute& attr , mpl::false_) {
            return parse_rhs_main(rhs, first, last, context, rcontext, attr);
        }

        template <typename RHS, typename Iterator , typename Context, typename RContext, typename ActualAttribute>
        static bool parse_rhs(RHS const& rhs , Iterator& first, Iterator const& last , Context const& context, RContext& rcontext, ActualAttribute& attr , mpl::true_) {
            return parse_rhs_main(rhs, first, last, context, rcontext, unused);
        }

        template <typename RHS, typename Iterator, typename Context , typename ActualAttribute, typename ExplicitAttrPropagation>
        static bool call_rule_definition(RHS const& rhs , char const* rule_name , Iterator& first, Iterator const& last , Context const& context, ActualAttribute& attr , ExplicitAttrPropagation) {
            typedef traits::make_attribute<Attribute, ActualAttribute> make_attribute;

            // do down-stream transformation, provides attribute for
            // rhs parser
            typedef traits::transform_attribute< typename make_attribute::type, Attribute, parser_id>
            transform;

            typedef typename make_attribute::value_type value_type;
            typedef typename transform::type transform_attr;
            value_type made_attr = make_attribute::call(attr);
            transform_attr attr_ = transform::pre(made_attr);

            bool ok_parse
              //Creates a place to hold the result of parse_rhs
              //called inside the following scope.
              ;
            {
             // Create a scope to cause the dbg variable below (within
             // the #if...#endif) to call it's DTOR before any
             // modifications are made to the attribute, attr_ passed
             // to parse_rhs (such as might be done in
             // traits::post_transform when, for example,
             // ActualAttribute is a recursive variant).

                ok_parse = parse_rhs(rhs, first, last, context, attr_, attr_ , mpl::bool_ < (  RHS::has_action && !ExplicitAttrPropagation::value ) >() );
            }
            if (ok_parse) {
                // do up-stream transformation, this integrates the results
                // back into the original attribute value, if appropriate
                traits::post_transform(attr, std::forward<transform_attr>(attr_));
            }
            return ok_parse;
        }
    };
}}
