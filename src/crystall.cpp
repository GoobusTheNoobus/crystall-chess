#include <iostream>

#include "eval.hpp"
#include "position.hpp"
#include "bitboards.hpp"
#include "uci.hpp"
#include "tt.hpp"
#include "search.hpp"

using namespace Crystall;

int main(void) {
    Bitboards::init();
    Evaluation::init();
    Zobrist::init();
    Search::init();
    
    std::cout << "Crystall UCI Chess Engine v1.6.0\n";

    UCI::loop();

    return 0;
}