/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
///////////////////////////////////////////////////////////////////////////////
//
//  Plain calculator example demonstrating the grammar. The parser is a
//  syntax checker only and does not do any semantic evaluation.
//
//  [ JDG May 10, 2002 ]        spirit 1
//  [ JDG March 4, 2007 ]       spirit 2
//  [ JDG February 21, 2011 ]   spirit 2.5
//  [ JDG June 6, 2014 ]        spirit x3
//
///////////////////////////////////////////////////////////////////////////////

#include "x4.hpp"

#include <iostream>
#include <string>
#include <list>
#include <numeric>

using namespace x4::literals;
using namespace boost::hana::literals;

namespace client
{

    ///////////////////////////////////////////////////////////////////////////////
    //  The calculator grammar
    ///////////////////////////////////////////////////////////////////////////////
    namespace calculator_grammar
    {
        template <class I>
        constexpr auto at(I i) {return boost::hana::reverse_partial(boost::hana::at, i);}


        X4_DECLARE(expression); // :: term
        X4_DECLARE(term);       // :: factor
        X4_DECLARE(factor);     // :: expression

        X4_DEFINE(expression) = term >> *(('+' >> term) | ('-' >> term));
            //% [](auto t, auto v) {
            //    for (auto i : v) {
            //        if (i.index() == 0) t += i[0_c][1_c];
            //        else t -= i[1_c][1_c];
            //    }
            //    return t;
            //};

        //X4_DEFINE(term) = seq(factor, *(('*' >> factor) | ('/' >> factor)));
        // % [](auto t, auto v) {
        //        for (auto i : v) {
        //            if (i.index() == 0) t *= i[0_c][1_c];
        //            else t /= i[1_c][1_c];
        //        }
        //        return t;
        //    };

        //X4_DEFINE(factor) = x4::uint_x
        //                  | ('(' >> expression >> ')') //% at(1_c)
        //                  | ('-' >> factor) //% at(1_c)
        //                  | ('+' >> factor); //% at(1_c);

        //X4_DEFINE(expression) = term >> *('-' >> term);

        X4_DEFINE(term) = factor >> *('*' >> factor);

        X4_DEFINE(factor) = x4::uint_x
                          | '(' >> expression >> ')'
                          | ('+' >> factor);

        auto calculator = factor;
    }

    using calculator_grammar::calculator;

}

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////
int main() {

    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "Expression parser...\n\n";
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "Type an expression...or [q or Q] to quit\n\n";

    std::string str = "1";
    while (std::getline(std::cin, str))
    {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        auto& calc = client::calculator;    // Our grammar
        bool r = parser(client::calculator, ' '_x).match(str);


        if (r) {
            //auto p = parser(client::calculator, ' '_x)(str);// << std::endl;
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            std::cout << "-------------------------\n";
        } else {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
        }
    }

    std::cout << "Bye... :-) \n\n";
    return 0;
}
