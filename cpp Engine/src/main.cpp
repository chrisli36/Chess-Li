#include <iostream>

#include "board.hpp"
#include "move.hpp"

// cd build
// cmake ..
// make
// ./chess-engine

int main() {
    Board board;
    board.print();

    std::vector<Move> moves = board.get_moves();
    for (const Move& move : moves) {
        std::cout << move.to_string() << "\n";
    }

    return 0;
}