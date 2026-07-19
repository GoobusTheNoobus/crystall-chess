// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#include <iostream>

#include "eval.hpp"
#include "zobrist.hpp"
#include "attacks.hpp"
#include "uci.hpp"
#include "search.hpp"

using namespace Crystall;

void initialize() {

    // Initialize everything
    Attacks::init();
    Evaluation::init();
    Zobrist::init();
    Search::init();
}

int main(void) {
    initialize();
    std::cout << "Crystall UCI Chess Engine v1.6.3\n";

    UCI::loop();

    return 0;
}