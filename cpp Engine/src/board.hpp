#pragma once

#include <string>
#include <array>
#include "piece.hpp"
#include "bitboard.hpp"
#include "castling_rights.hpp"
#include "turn.hpp"
#include "move.hpp"

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

    /**
     * @brief Calculates valid moves for the current position.
     * 
     * @return A vector of valid moves.
     */
    std::vector<Move> get_moves() const;

private:
    // VARIABLES
    // board state variables
    Piece squares[64];
    Bitboard piece_bitboards[2][6];
    Bitboard color_bitboards[2];
    Bitboard all_pieces_bitboard;
    CastlingRights castling_rights;

    Turn turn;
    Bitboard en_passant_square;

    // turn specific variables
    bool inited = false;
    bool castle_king, castle_queen;
    Bitboard* friends;
    Bitboard* enemies;
    Bitboard (*friend_arr)[6];
    Bitboard (*enemy_arr)[6];

    // METHODS
    void reset();
    void update_turn();
};