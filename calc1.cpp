#include "xf.hpp"

#include <iostream>
#include <string>
#include <list>
#include <numeric>
#include <boost/hana/functional/arg.hpp>

using namespace xf::literals;
using namespace boost::hana::literals;
namespace hana = boost::hana;

namespace client {

    ///////////////////////////////////////////////////////////////////////////////
    //  The calculator grammar
    ///////////////////////////////////////////////////////////////////////////////

    XF_DECLARE(expression); // :: term
    XF_DECLARE(term);       // :: factor
    XF_DECLARE(factor);     // :: expression

    //  ('+' >> term) --> void(int &r)
    auto plus = seq('+'_x, term)   % xf::partial % [](auto, auto t, auto &r) {r += t;}; // seq : return a sequence of the terms, equivalent to >>
    auto sub  = seq('-'_x, term)   % xf::partial % [](auto, auto t, auto &r) {r -= t;}; // sequence unpacks its arguments into the % chained function
    auto mult = seq('*'_x, factor) % xf::partial % [](auto, auto t, auto &r) {r *= t;}; // '*'_x doesn't have suppressed output right now (hence auto, )
    auto div  = seq('/'_x, factor) % xf::partial % [](auto, auto t, auto &r) {r /= t;}; // partial binds the arguments to the next % chained function (syntax maybe not great)

    // (term >> *(plus | sub)) --> int
    XF_DEFINE(expression) = seq(term, *any(plus, sub))           // any is equivalent to |, * returns vector by default
        % [](auto t, auto ops) {for (auto op : ops) op(t); return t;}; // the curried functions can be called with the last argument alone now

    // (factor >> *(mult | div)) --> int
    XF_DEFINE(term) = seq(factor, *any(mult, div))
        % [](auto t, auto ops) {for (auto op : ops) op(t); return t;};

    // (uint | '(' >> expression >> ')' | '+' >> factor | '-' >> factor) --> int
    XF_DEFINE(factor) = (xf::uint_x                                              // simple unsigned int
                      | seq('(', expression, ')') % hana::arg<2>                 // take second argument
                      | seq('+', factor)          % hana::arg<2>                 // take second argument
                      | seq('-', factor)          % hana::arg<2> % hana::negate) // take second argument and negate it
                      % xf::static_cast_c<int>;                                  // my variant has a cast operator

    // Recursion is tricky sometimes because accessing incomplete types is disallowed
    // However here the type information is sufficient with <int>
    // Also, the example works with <double> too!

    auto calculator = expression;
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
            auto p = parser(client::calculator, ' '_x)(str);

            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded: " << p << "\n";
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
