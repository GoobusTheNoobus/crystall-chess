#include <iostream>

#include "eval.hpp"
#include "position.hpp"
#include "bit.hpp"
#include "uci.hpp"
#include "tt.hpp"

using namespace Crystall;

int main(void) {
    Bitboards::init();
    Evaluation::init();
    Zobrist::init();
    
    std::cout << "Crystall UCI Chess Engine v1.5.0\n";

    UCI::loop();

    return 0;
}