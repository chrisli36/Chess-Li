#include "board.hpp"
#include <iostream>
#include <iomanip>

Board::Board(std::string fen) {
    // Initialize the board from the FEN string
    int i = 0;
    for (char c : fen) {
        if (i == 64) {
            break;
        }
        if (c == '/') {
            continue;
        }
        if (std::isdigit(c)) {
            for (int j = i; j < i + (c - '0'); ++j) {
                squares[j] = Piece::EMPTY;
            }
            i += (c - '0');
        } else {
            squares[i] = Piece::fen_to_piece(c);
            i++;
        }
    }
}

void Board::print() const {
    for (int rank = 0; rank < 8; ++rank) {
        std::cout << "  " << std::string(33, '-') << "\n";
        std::cout << 8 - rank << " |";
        for (int file = 0; file < 8; ++file) {
            std::cout << " " << Piece::piece_to_str(squares[rank * 8 + file]) << " |";
        }
        std::cout << "\n";
    }
    std::cout << "  " << std::string(33, '-') << "\n";
    std::cout << "    a   b   c   d   e   f   g   h\n";
}