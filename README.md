# Crystall 
## Description
Crystall is a UCI-supporting hobby chess engine. It is written in C++ and developed by @GoobusTheNoobus. It uses bitboards and a mailbox for position representation, bitboards for very fast move generation, and an alpha-beta negamax search framework.

## Building
Crystall uses makefiles to compile its code. Navigate to project root, and use `make` to build into an executable `crystall.exe` located in the same directory

## Features
As of version 1.2.0, here are the features:
- Position representation of mailbox and bitboards ([`position.hpp`](src/position.hpp))
- Pseudolegal move generation ([`position.cpp`](src/position.cpp), [`bit.cpp`](src/bit.cpp))
- UCI communication loop ([`uci.cpp`](src/uci.cpp))
- Knowledge of all standard chess rules
- Alpha Beta Search ([`search.cpp`](src/search.cpp))
    - Quiescence Search 
        - Delta Pruning
    - Iterative Deepening
- Evaluation
    - Material + PST




