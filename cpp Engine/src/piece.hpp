#pragma once
#include <cstdint>

#include "turn.hpp"

struct Piece {
    enum PieceType : uint8_t {
        PAWN   = 0,
        KNIGHT = 1,
        BISHOP = 2,
        ROOK   = 3,
        QUEEN  = 4,
        KING   = 5,
        EMPTY  = 6,
        WHITE  = 1 << 3,
        BLACK  = 1 << 4,

        WHITE_PAWN   = PAWN   | WHITE,
        WHITE_KNIGHT = KNIGHT | WHITE,
        WHITE_BISHOP = BISHOP | WHITE,
        WHITE_ROOK   = ROOK   | WHITE,
        WHITE_QUEEN  = QUEEN  | WHITE,
        WHITE_KING   = KING   | WHITE,
        BLACK_PAWN   = PAWN   | BLACK,
        BLACK_KNIGHT = KNIGHT | BLACK,
        BLACK_BISHOP = BISHOP | BLACK,
        BLACK_ROOK   = ROOK   | BLACK,
        BLACK_QUEEN  = QUEEN  | BLACK,
        BLACK_KING   = KING   | BLACK
    } type;

    static constexpr uint8_t PIECE_MASK = 0b111;
    static constexpr uint8_t WHITE_MASK = 0b1000;
    static constexpr uint8_t BLACK_MASK = 0b10000;

    /**
     * @brief Constructs a new Piece object.
     * 
     * @param t The type of the piece (e.g., WHITE_PAWN, BLACK_KNIGHT)
     */
    constexpr Piece(PieceType t = EMPTY) : type(t) {}

    /**
     * @brief Resets the piece to EMPTY.
     * 
     */
    constexpr void reset() { type = EMPTY; }

    /** 
     * @brief Gets PieceType of this piece.
     * 
     * @return The PieceType of this piece.
     */
    constexpr PieceType get_piece() const { return static_cast<PieceType>(type & PIECE_MASK); }

    /** 
     * @brief Gets the color of this piece.
     * 
     * @return The color of this piece (WHITE or BLACK).
     */
    constexpr Turn get_color() const { return static_cast<Turn>((type & BLACK_MASK) > 0); }


    /**
     * @brief  Checks if this piece is empty.
     * 
     * @return true if the piece is empty, false otherwise.
     */
    constexpr bool is_empty() const { return type == EMPTY; }

    /**
     * @brief Checks if this piece is an enemy piece.
     * 
     * @return true if the piece is an enemy, false otherwise.
     */
    constexpr bool is_enemy(Turn turn) const { return !is_empty() && get_color() != turn;}

    /**
     * @brief Checks if this piece is a friend piece.
     * 
     * @return true if the piece is a friend, false otherwise.
     */
    constexpr bool is_friendly(Turn turn) const { return !is_empty() && get_color() == turn;}

    /**
     * @brief  Converts a FEN character into a Piece.
     * 
     * @param c the FEN character (e.g., 'p', 'K')
     * @return A Piece corresponding to the FEN character.
     */
    static constexpr Piece from_fen(char c) {
        switch (c) {
            case 'p': return PieceType(PAWN | BLACK);
            case 'n': return PieceType(KNIGHT | BLACK);
            case 'b': return PieceType(BISHOP | BLACK);
            case 'r': return PieceType(ROOK | BLACK);
            case 'q': return PieceType(QUEEN | BLACK);
            case 'k': return PieceType(KING | BLACK);
            case 'P': return PieceType(PAWN | WHITE);
            case 'N': return PieceType(KNIGHT | WHITE);
            case 'B': return PieceType(BISHOP | WHITE);
            case 'R': return PieceType(ROOK | WHITE);
            case 'Q': return PieceType(QUEEN | WHITE);
            case 'K': return PieceType(KING | WHITE);
            default : return PieceType(EMPTY);
        }
    }

    static constexpr char representations[7] = {'P', 'N', 'B', 'R', 'Q', 'K', '.'};
    /**
     * Converts this Piece into its FEN character.
     *
     * @return The FEN character representing this piece (e.g., 'p', 'N').
     */
    constexpr char to_char() const {
        if (type == EMPTY) return '.';
        return representations[type & 0x7] + ((type & BLACK) ? 'a' - 'A' : 0);
    }
};