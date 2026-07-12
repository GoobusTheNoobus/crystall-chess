#include <iostream>

#include "engine/eval/eval.hpp"
#include "chess/board/zobrist.hpp"
#include "chess/bitboards/bitboards.hpp"
#include "protocol/uci.hpp"
#include "engine/search/search.hpp"

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