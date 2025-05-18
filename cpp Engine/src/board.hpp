#pragma once

#include <string>
#include <array>
#include <vector>

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
    std::vector<Move> get_moves();

    /**
     * @brief Makes the move on the board, assumes a valid move.
     * 
     * @param move The move to be made 
     */
    void make_move(const Move* move);

    /** 
     * @brief Returns the piece at a given square.
     * 
     * @param file The file of the square (0-7).
     * @param rank The rank of the square (0-7).
     * @return The piece at the square.
     */
    constexpr Piece get_piece(const int file, const int rank) const {
        return squares[rank * 8 + file];
    }

    /**
     * @brief Returns if the square is empty.
     * 
     * @param sq The square on the board.
     * @return A boolean indicating if the square is empty.
     */
    constexpr bool is_empty(const int sq) const {
        return squares[sq].is_empty();
    }

    /** 
     * @brief Returns the piece at a given square.
     * 
     * @param sq The square (0-63).
     * @return The piece at the square.
     */
    constexpr Piece get_piece(int sq) const {
        return squares[sq];
    }

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
    Bitboard* friend_arr;
    Bitboard* enemy_arr;

    std::vector<Move> moves;

    // METHODS
    void reset();
    void update_turn();
    bool is_valid_fr(const int file, const int rank, int* sq);

    // MOVE GENERATION
    void pawn_moves(const uint8_t sq);
    void knight_moves(const uint8_t sq);
    void bishop_moves(const uint8_t sq);
    void rook_moves(const uint8_t sq);
    void queen_moves(const uint8_t sq);
    void king_moves(const uint8_t sq);

    // CONSTANTS
    static constexpr Piece::PieceType PIECES[] = { Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN, Piece::KING };
    static constexpr int KNIGHT_DIRECTIONS[8][2] = {
        { 1,  2}, { 2,  1}, { 2, -1}, { 1, -2},
        {-1, -2}, {-2, -1}, {-2,  1}, {-1,  2}
    };
    static constexpr int BISHOP_DIRECTIONS[4][2] = {
        { 1,  1}, { 1, -1}, {-1, -1}, {-1,  1}
    };
    static constexpr int ROOK_DIRECTIONS[4][2] = {
        { 1,  0}, { 0,  1}, {-1,  0}, { 0, -1}
    };
    static constexpr void (Board::*CALCULATE_MOVES_FUNCTIONS[])(uint8_t) = {
        &Board::pawn_moves,
        &Board::knight_moves,
        &Board::bishop_moves,
        &Board::rook_moves,
        &Board::queen_moves,
        &Board::king_moves
    };

};