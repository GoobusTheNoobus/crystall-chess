#include <iostream>

#include "position.hpp"
#include "bit.hpp"
#include "movelist.hpp"

using namespace Crystall;

int main(void) {
    Bitboards::init();

    std::cout << "Crystall UCI Chess Engine v0.1.0\n";

    Position pos;
    pos.parse_fen("r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
    MoveList list(pos);
    
    std::cout << list.to_string();

    return 0;
}