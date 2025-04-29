#pragma once
#include <cstdint>

struct Piece {
    enum PieceType : uint8_t {
        EMPTY  = 0,
        PAWN   = 1,
        KNIGHT = 2,
        BISHOP = 3,
        ROOK   = 4,
        QUEEN  = 5,
        KING   = 6,
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
    };

    PieceType type;

    /**
     * @brief Constructs a new Piece object.
     * 
     * @param t The type of the piece (e.g., WHITE_PAWN, BLACK_KNIGHT)
     */
    constexpr Piece(PieceType t = EMPTY) : type(t) {}

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

    static constexpr char representations[7] = {'.', 'P', 'N', 'B', 'R', 'Q', 'K'};

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