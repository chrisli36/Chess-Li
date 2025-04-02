#ifndef PIECE_HPP
#define PIECE_HPP

class Piece {
public:
    enum PieceType {
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

    static PieceType fen_to_piece(char c) {
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

    static char piece_to_str(PieceType c) {
        return pieces[c & 0x7] + ((c & BLACK) ? 'a' - 'A' : 0);
    }

private:
    static const char pieces[7];
};

#endif // PIECE_HPP