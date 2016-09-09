#define CATCH_CONFIG_CONSOLE_WIDTH 100 // width of Catch output in terminal
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

int main(int argc, char* const argv[]) {return Catch::Session().run(argc, argv);}
