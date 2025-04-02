#ifndef BOARD_HPP
#define BOARD_HPP

#include <string>
#include <array>
#include "piece.hpp"

class Board {
public:
    Board(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    void print() const;

private:
    std::array<Piece::PieceType, 64> squares;

};

#endif // BOARD_HPP