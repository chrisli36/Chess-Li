#pragma once

#include <string>
#include <array>
#include "piece.hpp"
#include "bitboard.hpp"
#include "castling_rights.hpp"

class Board {
public:
    /**
     * @brief Constructs a new Board object from a FEN string.
     * 
     * @param fen
     */
    Board(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    /**
     * @brief Prints the board to the console.
     * 
     */
    void print() const;

private:
    // board state variables
    Piece squares[64];
    Bitboard piece_bitboards[2][6];
    Bitboard color_bitboards[2];
    Bitboard all_pieces_bitboard;
    CastlingRights castling_rights;

    enum Turn : uint8_t {
        WHITE = 0,
        BLACK = 1
    } turn;

    Bitboard en_passant_square;

    // turn specific variables
    int* castle_king, castle_queen;
    Bitboard* friends, enemies;
    Bitboard* friend_arr, enemy_arr;
};