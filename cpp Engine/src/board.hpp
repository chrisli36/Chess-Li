#pragma once

#include <string>
#include <vector>

#include "piece.hpp"
#include "bitboard.hpp"
#include "castling_rights.hpp"
#include "position.hpp"
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
    constexpr Piece get_piece(const int sq) const {
        return squares[sq];
    }

    constexpr bool is_controlled(const int sq) const {
        return controlled_squares.covers(sq);
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
    Bitboard controlled_squares;
    uint8_t attackers[2];
    uint8_t attacker_count;
    Bitboard pinned_limits[64];
    Bitboard evasion_mask;

    std::vector<Move> moves;

    // Macro for iterating over sliding piece directions
    #define MOVE_ITERATOR(dir, rank, file, new_rank, new_file, new_sq) \
        for (new_rank = rank + dir[0], new_file = file + dir[1]; \
            is_valid_fr(new_file, new_rank, &new_sq); \
            new_rank += dir[0], new_file += dir[1])

    // METHODS
    void reset();
    void update_turn();
    bool is_valid_fr(const int file, const int rank, int* sq);
    void erase_piece(const int sq);
    void add_piece(const int sq, const Piece piece);
    void rook_disabling_castling_move(const uint8_t sq);
    void add_king_attacker(const uint8_t start, Bitboard attacks);
    bool is_sliding_piece(const Piece piece);
    void calculate_pins();
    bool can_move_under_pin(const uint8_t sq, const uint8_t new_sq);
    bool two_pawns_en_passant(const int file, const int rank, const int dir[2], int* two_pawns_sq);
    bool is_aligned(const int dir[2], const Piece piece);

    // MOVE GENERATION
    void pawn_controlled(const uint8_t sq);
    void knight_controlled(const uint8_t sq);
    void bishop_controlled(const uint8_t sq);
    void rook_controlled(const uint8_t sq);
    void queen_controlled(const uint8_t sq);
    void king_controlled(const uint8_t sq);

    void pawn_moves(const uint8_t sq);
    void knight_moves(const uint8_t sq);
    void bishop_moves(const uint8_t sq);
    void rook_moves(const uint8_t sq);
    void queen_moves(const uint8_t sq);
    void king_moves(const uint8_t sq);

    // CONSTANTS
    static constexpr Piece::PieceType PIECES[] = { Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN, Piece::KING };
    static constexpr int BISHOP_DIRECTIONS[4][2] = {
        { 1,  1}, { 1, -1}, {-1, -1}, {-1,  1}
    };
    static constexpr int ROOK_DIRECTIONS[4][2] = {
        { 1,  0}, { 0,  1}, {-1,  0}, { 0, -1}
    };
    static constexpr int KING_DIRECTIONS[8][2] = {
        { 1,  1}, { 1,  0}, { 1, -1}, { 0, -1},
        {-1, -1}, {-1,  0}, {-1,  1}, { 0,  1}
    };
    static constexpr void (Board::*CALCULATE_MOVES_FUNCTIONS[])(uint8_t) = {
        &Board::pawn_moves,
        &Board::knight_moves,
        &Board::bishop_moves,
        &Board::rook_moves,
        &Board::queen_moves,
        &Board::king_moves
    };
    static constexpr void (Board::*CALCULATE_CONTROLLED_FUNCTIONS[])(uint8_t) = {
        &Board::pawn_controlled,
        &Board::knight_controlled,
        &Board::bishop_controlled,
        &Board::rook_controlled,
        &Board::queen_controlled,
        &Board::king_controlled
    };
    static constexpr Bitboard KINGSIDE_CASTLE[2] = {
        Bitboard(1ULL << F1) | Bitboard(1ULL << G1),
        Bitboard(1ULL << F8) | Bitboard(1ULL << G8),
    };
    static constexpr Bitboard QUEENSIDE_CASTLE[2] = {
        Bitboard(1ULL << D1) | Bitboard(1ULL << C1) | Bitboard(1ULL << B1),
        Bitboard(1ULL << D8) | Bitboard(1ULL << C8) | Bitboard(1ULL << B8),
    };
};