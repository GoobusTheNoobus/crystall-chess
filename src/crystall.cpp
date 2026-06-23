#include <iostream>

#include "position.hpp"
#include "bit.hpp"
#include "movelist.hpp"
#include "search.hpp"
#include "uci.hpp"

using namespace Crystall;

int main(void) {
    Bitboards::init();
    std::cout << "Crystall UCI Chess Engine v1.0.0\n";

    UCI::loop();

    return 0;
}