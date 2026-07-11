#include <iostream>

#include "eval.hpp"
#include "position.hpp"
#include "bitboards.hpp"
#include "uci.hpp"
#include "search.hpp"

using namespace Crystall;

void initialize() {

    // Initialize everything
    Bitboards::init();
    Evaluation::init();
    Zobrist::init();
    Search::init();
}

int main(void) {
    initialize();
    std::cout << "Crystall UCI Chess Engine v1.6.1\n";

    UCI::loop();

    return 0;
}